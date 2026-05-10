#include "client.h"
#include <arpa/inet.h>   // 提供 inet_aton、inet_ntop
#include <sys/socket.h>  // 提供 socket、connect、getsockopt
#include <netinet/tcp.h> // 提供 TCP_INFO、struct tcp_info
#include <netdb.h>       // 提供 gethostbyname、struct hostent
#include <string.h>      // 提供 memset、strncpy
#include <errno.h>       // 提供 errno、strerror
#include <stdio.h>       // 提供 printf
#include <unistd.h>      // 提供 close

/**
 * @brief 域名解析（线程安全版，支持IPv4）
 * @param domain 输入：待解析的域名（如 "www.example.com"）
 * @param ipaddr 输出：存储解析后的IPv4地址（需确保至少16字节空间，适配 "xxx.xxx.xxx.xxx\0"）
 * @return 0 成功，-1 参数无效，-2 解析失败（如域名不存在、网络不可达）
 */
int socket_resolver(const char *domain, char* ipaddr)
{
    struct hostent* host = NULL;
    struct in_addr addr; // 临时存储IPv4地址，避免依赖inet_ntoa的静态缓冲区

    // 1. 参数有效性检查
    if (!domain || !ipaddr)
    {
        printf("[socket_resolver] Error: Invalid parameters (domain=%p, ipaddr=%p)\n", 
               domain, ipaddr);
        return -1;
    }

    // 2. 域名解析（gethostbyname已过时，建议后续替换为getaddrinfo以支持IPv6）
    host = gethostbyname(domain);
    if (!host)
    {
        printf("[socket_resolver] Error: Resolve domain=%s failed (h_errno=%d, reason=%s)\n", 
               domain, h_errno, hstrerror(h_errno)); // 打印具体错误原因
        return -2;
    }

    // 3. 提取第一个IPv4地址（host->h_addr_list存储多个地址，优先取第一个）
    memcpy(&addr, host->h_addr_list[0], sizeof(addr));
    // 4. 线程安全的IP地址转换（inet_ntop支持IPv4/IPv6，使用用户提供的缓冲区）
    inet_ntop(AF_INET, &addr, ipaddr, 16); // 16字节足够存储IPv4地址+结束符

    printf("[socket_resolver] Success: Domain=%s -> IP=%s\n", domain, ipaddr);
    return 0;
}

/**
 * @brief 初始化socket结构体（绑定服务器IP/域名和端口，设置初始状态）
 * @param sock 输入输出：待初始化的socket_t结构体指针
 * @param host 输入：服务器IP地址或域名（如 "192.168.1.100" 或 "www.example.com"）
 * @param port 输入：服务器端口（主机字节序，范围1-65535）
 * @return 0 成功，-1 参数无效（如sock为NULL、端口非法）
 */
int socket_init(socket_t *sock, char *host, int port)
{
    // 1. 参数有效性检查（端口范围：1-65535，0为无效端口）
    if (!sock || port <= 0 || port > 65535)
    {
        printf("[socket_init] Error: Invalid parameters (sock=%p, port=%d)\n", 
               sock, port);
        return -1;
    }

    // 2. 清空结构体，初始化默认状态（未连接）
    memset(sock, 0, sizeof(*sock));
    sock->fd = -1;       // 初始化为无效文件描述符（避免误判为有效fd）
    sock->connected = 0; // 初始化为未连接状态
    sock->port = port;   // 存储主机字节序的端口

    // 3. 若传入主机地址/域名，执行解析（解析失败不影响初始化，后续连接时会报错）
    if (host)
    {
        if (socket_resolver(host, sock->host) != 0)
        {
            printf("[socket_init] Warning: Resolve host=%s failed, will retry when connecting\n", 
                   host);
        }
    }

    printf("[socket_init] Success: Socket initialized (host=%s, port=%d)\n", 
           sock->host, sock->port);
    return 0;
}

/**
 * @brief 关闭socket并释放资源（确保文件描述符和连接状态正确重置）
 * @param sock 输入输出：待关闭的socket_t结构体指针
 * @return 0 成功，-1 参数无效
 */
int socket_close(socket_t *sock)
{
    if (!sock)
    {
        printf("[socket_close] Error: Invalid parameter (sock=%p)\n", sock);
        return -1;
    }

    // 仅关闭有效的文件描述符（避免重复关闭导致的错误）
    if (sock->fd > 0)
    {
        printf("[socket_close] Info: Closing socket fd=%d\n", sock->fd);
        close(sock->fd);       // 关闭socket文件描述符
        sock->fd = -1;         // 标记为无效fd，防止后续误操作
        sock->connected = 0;   // 重置为未连接状态
    }

    return 0;
}

/**
 * @brief 建立TCP连接（自动关闭旧连接，创建新连接）
 * @param sock 输入输出：socket_t结构体指针（需先通过socket_init初始化）
 * @return 0 成功，-1 参数无效，-2 创建socket失败，-3 连接失败（如服务器不可达、端口未开放）
 */
int socket_connect(socket_t *sock)
{
    struct sockaddr_in serv_addr;
    int conn_fd = -1; // 临时存储新创建的socket fd（避免直接操作sock->fd）

    // 1. 参数有效性检查
    if (!sock)
    {
        printf("[socket_connect] Error: Invalid parameter (sock=%p)\n", sock);
        return -1;
    }

    // 2. 先关闭已有的连接（防止旧连接未释放导致资源泄漏）
    socket_close(sock);

    // 3. 创建TCP socket（AF_INET=IPv4，SOCK_STREAM=TCP，0=默认协议）
    conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_fd < 0)
    {
        printf("[socket_connect] Error: Create socket failed (errno=%d, reason=%s)\n", 
               errno, strerror(errno));
        return -2;
    }

    // 4. 初始化服务器地址结构体
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;                // 地址族：IPv4
    serv_addr.sin_port = htons(sock->port);        // 端口转换为网络字节序（大端）
    // IP地址转换为网络字节序（需确保sock->host已解析为有效IPv4）
    if (inet_aton(sock->host, &serv_addr.sin_addr) == 0)
    {
        printf("[socket_connect] Error: Invalid server IP=%s\n", sock->host);
        close(conn_fd); // 连接前失败，关闭临时fd（关键修复！避免资源泄漏）
        return -3;
    }

    // 5. 发起TCP连接（超时由系统默认控制，如需自定义超时需额外设置SO_SNDTIMEO）
    // printf("[socket_connect] Info: Connecting to %s:%d (fd=%d)...\n", 
    //        sock->host, sock->port, conn_fd);
    if (connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        // printf("[socket_connect] Error: Connect failed (errno=%d, reason=%s)\n", 
        //        errno, strerror(errno));
        close(conn_fd); // 连接失败，关闭临时fd（关键修复！）
        return -3;
    }

    // 6. 连接成功：更新socket结构体状态
    sock->fd = conn_fd;       // 保存有效fd
    sock->connected = 1;      // 标记为已连接状态
    printf("[socket_connect] Success: Connected to %s:%d (fd=%d)\n", 
           sock->host, sock->port, sock->fd);

    return 0;
}

/**
 * @brief 检查socket连接状态（基于TCP底层状态机，支持状态变化检测）
 * @param sock 输入输出：socket_t结构体指针
 * @return 1 已连接（TCP_ESTABLISHED状态），0 未连接，-1 参数无效
 */
int socket_check_connect(socket_t *sock)
{
    struct tcp_info info;
    socklen_t len = sizeof(info);
    int changed = 0; // 标记连接状态是否发生变化

    // 1. 参数有效性检查
    if (!sock)
    {
        // printf("[socket_check_connect] Error: Invalid parameter (sock=%p)\n", sock);
        return -1;
    }

    // 2. 情况1：fd无效（直接判定为未连接）
    if (sock->fd < 0)
    {
        changed = (sock->connected == 1) ? 1 : 0; // 若之前是连接状态，标记为变化
        sock->connected = 0;
        goto out; // 跳转到状态处理逻辑
    }

    // 3. 情况2：通过getsockopt获取TCP状态（需检查调用结果，避免无效状态）
    if (getsockopt(sock->fd, IPPROTO_TCP, TCP_INFO, &info, &len) < 0)
    {
        // printf("[socket_check_connect] Error: Get TCP info failed (fd=%d, errno=%d, reason=%s)\n", 
        //        sock->fd, errno, strerror(errno));
        changed = (sock->connected == 1) ? 1 : 0;
        sock->connected = 0;
        goto out;
    }

    // 4. 情况3：根据TCP状态判断（仅TCP_ESTABLISHED为有效连接状态）
    if (info.tcpi_state == TCP_ESTABLISHED)
    {
        changed = (sock->connected == 0) ? 1 : 0; // 若之前是未连接，标记为变化
        sock->connected = 1;
    }
    else
    {
        changed = (sock->connected == 1) ? 1 : 0; // 若之前是连接，标记为变化
        sock->connected = 0;
        printf("[socket_check_connect] Warning: TCP state is not ESTABLISHED (fd=%d, state=%d)\n", 
               sock->fd, info.tcpi_state); // 打印具体状态码，便于调试
    }

out:
    // 5. 打印状态变化日志（便于定位连接断开/恢复时机）
    if (changed)
    {
        // printf("[socket_check_connect] Info: Socket status changed to %s (fd=%d)\n", 
        //        sock->connected ? "CONNECTED" : "DISCONNECTED", sock->fd);
    }

    return sock->connected;
}