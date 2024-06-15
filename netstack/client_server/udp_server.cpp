#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
    // 将指定网卡的 IP 地址填入
    const char *ip_address = "192.168.133.138"; // 替换为你的网卡 IP 地址
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

    printf("UDP Server is listening on %s:%d\n", ip_address, PORT);

    // 接收数据
    while (1) {
        ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
        if (len < 0) {
            perror("Receive failed");
            break;
        }
        buffer[len] = '\0'; // Null-terminate received data
        printf("Received: %s\n", buffer);

        // 发送响应
        sendto(sockfd, buffer, len, 0, (struct sockaddr*)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}
