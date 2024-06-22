#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#define debugsit    std::cout << "[debug][" << __FUNCTION__ << ":" << __LINE__ << "]:" << std::endl;


#define SERVER_IP       "192.168.133.140"
#define SERVER_PORT      8080

void print_client_info(struct sockaddr_in* p) {
    int port = htons(p->sin_port);

    char ip[16];
    memset(ip, 0, sizeof(ip));

    inet_ntop(AF_INET, &(p->sin_addr.s_addr), ip, sizeof(ip));

    printf("client connected: %s(%d)\n", ip, port);
}


int main() 
{
    int ser_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(ser_fd < 0) {
        perror("socket");
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    // addr.sin_addr.s_addr = htonl(INADDR_ANY);    /* 自动选择可用网卡 */

    /* 套接字绑定ip地址、端口号、协议族 */
    if(bind(ser_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
        perror("bind");
        close(ser_fd);
        return -1;
    }

    /* 进入监听状态 */
    if(listen(ser_fd, 10)) {
        perror("listen");
        close(ser_fd);
        return -1;
    }

    /* 端口复用 */
    int opt = 1;
    if (setsockopt(ser_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEPORT failed");
        close(ser_fd);
        return -1;
    }
    
    int epoll_handle = epoll_create(10);    /* 创建epoll句柄 */

    struct epoll_event ready_events[10];    /* 存放 epoll_wait 监控到的就绪事件 */
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; /* 可读事件 */
    event.data.fd = ser_fd; /* 监控服务端socket描述符，用来识别是哪个描述符 */

    /* 把监控描述符、事件注册进事件表中 */
    epoll_ctl(epoll_handle, EPOLL_CTL_ADD, ser_fd, &event);

    char buffer[1500];
    while(1) {
        int events = epoll_wait(epoll_handle, ready_events, 10, -1);
        // printf("events = %d.\n", events);
        for(int i = 0; i < events; ++i) {
            if(ready_events[i].data.fd == ser_fd) { /* 监听socket，处理客户端连接 */
                struct sockaddr_in client_addr;
                socklen_t addlen = sizeof(struct sockaddr_in);
                int client_fd = accept(ser_fd, (struct sockaddr *)&client_addr,     \
                            (socklen_t *__restrict )&addlen);
                
                if(client_fd < 0) { /* 客户端连接失败 */
                    perror("Accept failed");
                    continue;
                }

                print_client_info(&client_addr);

                fcntl(client_fd, F_SETFL, O_NONBLOCK);  /* 设置io为非阻塞状态 */
                event.events = EPOLLIN;
                event.data.fd = client_fd;
                epoll_ctl(epoll_handle, EPOLL_CTL_ADD, client_fd, &event); /* 注册事件 */
            } else {    /* io读 */
                // 处理客户端请求
                int sock_fd = ready_events[i].data.fd;
                ssize_t bytes_read = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    printf("Received from client %d: %s\n", sock_fd, buffer);
                    send(sock_fd, buffer, bytes_read, 0);
                } else if (bytes_read == 0) {
                    // 客户端断开连接
                    printf("Client disconnected: %d\n", sock_fd);
                    close(sock_fd);
                } else {
                    perror("Recv failed");
                    close(sock_fd);
                }

            }
        }

    }
}
