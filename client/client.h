#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>   // socket 相关函数
#include <netinet/in.h>   // sockaddr_in 结构体
#include <arpa/inet.h>    // inet_aton 函数
#include <errno.h>        // errno 错误码
#include <unistd.h>       // close 函数
#include<netinet/tcp.h>
#include <netinet/in.h>

typedef struct socket_s
{
	int 	fd;
	char	host[64];
	int		port;
	int     connected; 
}socket_t;

// 函数声明：第三个参数改为指针（传递地址）
int socket_resolver(const char *domain, char* ipaddr);
int socket_init(socket_t *sock, char *host, int port);
int socket_close(socket_t *sock);
int socket_connect(socket_t *sock);

int socket_check_connect(socket_t *sock);

#endif  // CLIENT_H