#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "database.h"
#include "gettime.h"
#include "logger.h"
#include "packet.h"
#include "socket.h"

#define DEFAULT_INTERVAL 3
#define CLIENT_DB_FILE "../etc/client.db"
#define JSON_BUF_SIZE 256

static void print_usage(char *progname)
{
    printf("%s usage:\n", progname);
    printf(" -i, --ipaddr <ip/domain>   server IP address or domain\n");
    printf(" -p, --port <port>          server port\n");
    printf(" -t, --time <seconds>       sample interval, default %d\n", DEFAULT_INTERVAL);
    printf(" -h, --help                 show this help\n");
}

static int send_packet(socket_t *sock, const packet_t *pack)
{
    char json[JSON_BUF_SIZE];
    int len;

    len = packet_to_json(pack, json, sizeof(json));
    if (len < 0)
    {
        log_error("packet_to_json failed: %d\n", len);
        return -1;
    }

    if (write(sock->fd, json, (size_t)len) != len)
    {
        log_warn("send packet failed: %s\n", strerror(errno));
        socket_close(sock);
        return -2;
    }

    log_info("send json: %s", json);
    return 0;
}

static void upload_cached_packets(sqlite3 *db, socket_t *sock)
{
    packet_t pack;

    while (database_count(db) > 0)
    {
        if (database_pop(db, &pack) < 0)
        {
            log_warn("read cached packet failed\n");
            break;
        }

        if (send_packet(sock, &pack) < 0)
        {
            database_insert(db, &pack);
            log_warn("reupload failed, packet saved again\n");
            break;
        }

        log_info("cached packet reuploaded\n");
    }
}

int main(int argc, char *argv[])
{
    sqlite3 *db = NULL;
    socket_t sock;
    packet_t pack;
    char *servip = NULL;
    int port = 0;
    int interval_time = DEFAULT_INTERVAL;
    time_t last_sample_time = 0;
    int ch;

    struct option opts[] = {
        {"ipaddr", required_argument, NULL, 'i'},
        {"port", required_argument, NULL, 'p'},
        {"time", required_argument, NULL, 't'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}};

    while ((ch = getopt_long(argc, argv, "i:p:t:h", opts, NULL)) != -1)
    {
        switch (ch)
        {
        case 'i':
            servip = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 't':
            interval_time = atoi(optarg);
            break;
        case 'h':
        default:
            print_usage(argv[0]);
            return 0;
        }
    }

    if (!servip || !port || interval_time <= 0)
    {
        print_usage(argv[0]);
        return -1;
    }

    log_open("console", LOG_LEVEL_DEBUG, 0, LOG_LOCK_DISABLE);

    if (database_open(&db, CLIENT_DB_FILE) < 0)
    {
        log_error("open database %s failed\n", CLIENT_DB_FILE);
        return -2;
    }

    socket_init(&sock, servip, port);
    log_info("client started, server=%s:%d interval=%d\n", servip, port, interval_time);

    while (1)
    {
        if (socket_check_connect(&sock) <= 0)
        {
            if (socket_connect(&sock) == 0)
            {
                log_info("connected to server %s:%d\n", sock.host, sock.port);
                upload_cached_packets(db, &sock);
            }
        }

        if (interval_timer(&last_sample_time, interval_time))
        {
            if (packet_sample(&pack) < 0)
            {
                log_warn("sample temperature failed\n");
                continue;
            }

            if (socket_check_connect(&sock) > 0)
            {
                if (send_packet(&sock, &pack) < 0)
                {
                    database_insert(db, &pack);
                    log_warn("network error, packet saved into sqlite\n");
                }
            }
            else
            {
                database_insert(db, &pack);
                log_warn("server offline, packet saved into sqlite\n");
            }
        }

        usleep(100000);
    }

    database_close(db);
    log_close();
    return 0;
}
