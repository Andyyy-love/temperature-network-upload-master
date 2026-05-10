#include "socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int socket_server_init(char *listen_ip, int listen_port)
{
    struct sockaddr_in servaddr;
    int rv = 0;
    int on = 1;
    int listenfd;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        printf("create TCP socket failure: %s\n", strerror(errno));
        return -1;
    }

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(listen_port);

    if (!listen_ip)
    {
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else if (inet_pton(AF_INET, listen_ip, &servaddr.sin_addr) <= 0)
    {
        rv = -2;
        goto cleanup;
    }

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        rv = -3;
        goto cleanup;
    }

    if (listen(listenfd, 13) < 0)
    {
        rv = -4;
        goto cleanup;
    }

cleanup:
    if (rv < 0)
    {
        close(listenfd);
        return rv;
    }

    return listenfd;
}
