#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "database.h"
#include "packet.h"
#include "socket.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define SERVER_DB_FILE "server.db"

static void print_usage(char *progname)
{
    printf("Usage: %s [OPTION]...\n", progname);
    printf(" -b, --daemon       run on background\n");
    printf(" -p, --port <port>  socket server port\n");
    printf(" -h, --help         show this help\n");
    printf("\nExample: %s -b -p 8900\n", progname);
}

static void add_client(struct pollfd *fds_array, int nfds, int *max, int connfd)
{
    int i;

    for (i = 1; i < nfds; i++)
    {
        if (fds_array[i].fd < 0)
        {
            fds_array[i].fd = connfd;
            fds_array[i].events = POLLIN;
            *max = i > *max ? i : *max;
            printf("accept new client[%d]\n", connfd);
            return;
        }
    }

    printf("client array full, refuse client[%d]\n", connfd);
    close(connfd);
}

static void handle_client_data(sqlite3 *db, struct pollfd *pfd)
{
    char buf[1024];
    packet_t pack;
    int rv;

    memset(buf, 0, sizeof(buf));
    rv = read(pfd->fd, buf, sizeof(buf) - 1);
    if (rv <= 0)
    {
        close(pfd->fd);
        pfd->fd = -1;
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
    int rv;
    int i;
    int max;
    struct pollfd fds_array[1024];

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

    listenfd = socket_server_init(NULL, serv_port);
    if (listenfd < 0)
    {
        printf("server listen on port %d failure\n", serv_port);
        return -2;
    }

    if (database_open(&db, SERVER_DB_FILE) < 0)
    {
        printf("open database %s failure\n", SERVER_DB_FILE);
        return -3;
    }

    if (daemon_run)
    {
        daemon(0, 0);
    }

    for (i = 0; i < (int)ARRAY_SIZE(fds_array); i++)
    {
        fds_array[i].fd = -1;
    }

    fds_array[0].fd = listenfd;
    fds_array[0].events = POLLIN;
    max = 0;

    printf("%s server listening on port %d\n", argv[0], serv_port);

    for (;;)
    {
        rv = poll(fds_array, max + 1, -1);
        if (rv < 0)
        {
            printf("poll failure: %s\n", strerror(errno));
            break;
        }

        if (fds_array[0].revents & POLLIN)
        {
            connfd = accept(listenfd, NULL, NULL);
            if (connfd >= 0)
            {
                add_client(fds_array, (int)ARRAY_SIZE(fds_array), &max, connfd);
            }
        }

        for (i = 1; i <= max; i++)
        {
            if (fds_array[i].fd < 0)
            {
                continue;
            }

            if (fds_array[i].revents & (POLLIN | POLLERR | POLLHUP))
            {
                handle_client_data(db, &fds_array[i]);
            }
        }
    }

    database_close(db);
    close(listenfd);
    return 0;
}
