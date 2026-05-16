#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "database.h"
#include "packet.h"
#include "socket.h"

#define MAX_EVENTS 1024
#define SERVER_DB_FILE "server.db"

static volatile sig_atomic_t g_running = 1;

static void handle_signal(int sig)
{
    (void)sig;
    g_running = 0;
}

static void print_usage(char *progname)
{
    printf("Usage: %s [OPTION]...\n", progname);
    printf(" -b, --daemon       run on background\n");
    printf(" -p, --port <port>  socket server port\n");
    printf(" -h, --help         show this help\n");
    printf("\nExample: %s -b -p 8900\n", progname);
}

static void handle_client_data(sqlite3 *db, int fd)
{
    char buf[1024];
    packet_t pack;
    int rv;

    memset(buf, 0, sizeof(buf));
    rv = read(fd, buf, sizeof(buf) - 1);
    if (rv <= 0)
    {
        close(fd);
        return;
    }

    if (packet_from_json(&pack, buf) < 0)
    {
        printf("invalid json packet: %s\n", buf);
        return;
    }

    printf("recv: id=%s time=%s temp=%.2f\n", pack.id, pack.time, pack.temperature);

    if (database_insert(db, &pack) < 0)
    {
        printf("write packet into database failed\n");
    }
}

int main(int argc, char **argv)
{
    sqlite3 *db = NULL;
    int listenfd;
    int connfd;
    int serv_port = 0;
    int daemon_run = 0;
    char *progname = basename(argv[0]);
    int opt;
    int epollfd;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    int nfds;
    int i;

    struct option long_options[] = {
        {"daemon", no_argument, NULL, 'b'},
        {"port", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}};

    while ((opt = getopt_long(argc, argv, "bp:h", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'b':
            daemon_run = 1;
            break;
        case 'p':
            serv_port = atoi(optarg);
            break;
        case 'h':
        default:
            print_usage(progname);
            return EXIT_SUCCESS;
        }
    }

    if (!serv_port)
    {
        print_usage(progname);
        return -1;
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    listenfd = socket_server_init(NULL, serv_port);
    if (listenfd < 0)
    {
        printf("server listen on port %d failure\n", serv_port);
        return -2;
    }

    if (database_open(&db, SERVER_DB_FILE) < 0)
    {
        printf("open database %s failure\n", SERVER_DB_FILE);
        close(listenfd);
        return -3;
    }

    if (daemon_run)
    {
        daemon(0, 0);
    }

    epollfd = epoll_create1(0);
    if (epollfd < 0)
    {
        printf("epoll_create failure: %s\n", strerror(errno));
        database_close(db);
        close(listenfd);
        return -4;
    }

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) < 0)
    {
        printf("epoll_ctl add listenfd failure: %s\n", strerror(errno));
        close(epollfd);
        database_close(db);
        close(listenfd);
        return -5;
    }

    printf("%s server listening on port %d (epoll mode)\n", argv[0], serv_port);

    while (g_running)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            printf("epoll_wait failure: %s\n", strerror(errno));
            break;
        }

        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == listenfd)
            {
                connfd = accept(listenfd, NULL, NULL);
                if (connfd >= 0)
                {
                    printf("accept new client[%d]\n", connfd);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = connfd;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
                    {
                        printf("epoll_ctl add client failure: %s\n", strerror(errno));
                        close(connfd);
                    }
                }
            }
            else
            {
                if (events[i].events & EPOLLIN)
                {
                    handle_client_data(db, events[i].data.fd);
                }
                if (events[i].events & (EPOLLERR | EPOLLHUP))
                {
                    printf("client[%d] error or hangup\n", events[i].data.fd);
                    close(events[i].data.fd);
                }
            }
        }
    }

    close(epollfd);
    close(listenfd);
    database_close(db);
    return 0;
}
