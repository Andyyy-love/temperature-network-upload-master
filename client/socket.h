#ifndef SOCKET_H
#define SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

typedef struct socket_s
{
    int fd;
    char host[64];
    int port;
    int connected;
} socket_t;

int socket_resolver(const char *domain, char *ipaddr);
int socket_init(socket_t *sock, char *host, int port);
int socket_close(socket_t *sock);
int socket_connect(socket_t *sock);
int socket_check_connect(socket_t *sock);

#endif
