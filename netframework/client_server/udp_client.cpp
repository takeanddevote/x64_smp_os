#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#define PORT 8080
#define SERVER_IP "192.168.133.138" // 服务器 IP
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;

    // 创建 UDP 套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // 发送数据
    const char* message = "Hello, UDP Server!";
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // 接收响应
    socklen_t addr_len = sizeof(server_addr);
    ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &addr_len);
    if (len < 0) {
        perror("Receive failed");
    } else {
        buffer[len] = '\0'; // Null-terminate received data
        std::cout << "Received from server: " << buffer << std::endl;
    }

    close(sockfd);
    return 0;
}
