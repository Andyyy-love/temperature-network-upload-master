#include "socket.h"

int socket_resolver(const char *domain, char *ipaddr)
{
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    struct sockaddr_in *addr = NULL;
    int rv;

    if (!domain || !ipaddr)
    {
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    rv = getaddrinfo(domain, NULL, &hints, &result);
    if (rv != 0 || !result)
    {
        return -2;
    }

    addr = (struct sockaddr_in *)result->ai_addr;
    inet_ntop(AF_INET, &addr->sin_addr, ipaddr, 16);
    freeaddrinfo(result);

    return 0;
}

int socket_init(socket_t *sock, char *host, int port)
{
    if (!sock || port <= 0 || port > 65535)
    {
        return -1;
    }

    memset(sock, 0, sizeof(*sock));
    sock->fd = -1;
    sock->connected = 0;
    sock->port = port;

    if (host)
    {
        strncpy(sock->domain, host, sizeof(sock->domain) - 1);
    }

    return 0;
}

int socket_close(socket_t *sock)
{
    if (!sock)
    {
        return -1;
    }

    if (sock->fd >= 0)
    {
        close(sock->fd);
    }

    sock->fd = -1;
    sock->connected = 0;
    return 0;
}

int socket_connect(socket_t *sock)
{
    struct sockaddr_in serv_addr;
    int conn_fd;

    if (!sock)
    {
        return -1;
    }

    socket_close(sock);

    if (socket_resolver(sock->domain, sock->host) != 0)
    {
        strncpy(sock->host, sock->domain, sizeof(sock->host) - 1);
    }

    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_fd < 0)
    {
        return -2;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(sock->port);

    if (inet_aton(sock->host, &serv_addr.sin_addr) == 0)
    {
        close(conn_fd);
        return -3;
    }

    if (connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        close(conn_fd);
        return -4;
    }

    sock->fd = conn_fd;
    sock->connected = 1;
    return 0;
}

int socket_check_connect(socket_t *sock)
{
    int error = 0;
    socklen_t len = sizeof(error);

    if (!sock || sock->fd < 0)
    {
        if (sock)
        {
            sock->connected = 0;
        }
        return 0;
    }

    if (getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0)
    {
        sock->connected = 0;
        return 0;
    }

    return sock->connected;
}
