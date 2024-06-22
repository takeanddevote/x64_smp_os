#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BACKLOG 5
#define BUFFER_SIZE 1024

int main() {
    int sockfd, new_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // 创建 TCP 套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // 配置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    // 指定网卡的 IP 地址
    const char *ip_address = "192.168.1.10"; // 替换为你要绑定的网卡 IP 地址
    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        return -1;
    }

    // 绑定套接字
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    // 开始监听
    if (listen(sockfd, BACKLOG) < 0) {
        perror("Listen failed");
        close(sockfd);
        return -1;
    }

    printf("Server listening on %s:%d\n", ip_address, PORT);

    // 接收连接
    while (1) {
        new_fd = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len);
        if (new_fd < 0) {
            perror("Accept failed");
            continue;
        }

        // 获取客户端 IP 地址
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("Connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        // 读取客户端数据
        while(1) {
            ssize_t bytes_read = read(new_fd, buffer, BUFFER_SIZE);
            if (bytes_read < 0) {
                perror("Read failed");
            } else {
                buffer[bytes_read] = '\0'; // Null-terminate the string
                printf("Received: %s\n", buffer);
            }
            write(new_fd, buffer, bytes_read);
        }

        // 关闭连接
        // close(new_fd);
    }

    close(sockfd);
    return 0;
}
