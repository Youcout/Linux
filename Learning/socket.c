#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 2048
#define BUFFER_SIZE 1024

void handle_client(int connfd)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(connfd, buffer, BUFFER_SIZE - 1, 0)) > 0)
    {
        buffer[bytes_received] = '\0'; // 确保字符串以null字符结尾
        printf("Received message: %s\n", buffer);

        // 构造回复信息
        ssize_t bytes_sent = send(connfd, buffer, bytes_received, 0);
        if (bytes_sent < 0)
        {
            perror("Send failed");
            break;
        }
    }

    if (bytes_received < 0)
    {
        perror("Receive failed");
    }

    close(connfd); // 关闭连接套接字
}

int main()
{
    int sockfd, connfd;                     // 文件描述符
    struct sockaddr_in serv_addr, cli_addr; // 地址结构
    socklen_t clilen;                       // 地址结构的大小

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 初始化服务器地址结构
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 绑定到所有可用的接口
    serv_addr.sin_port = htons(PORT);              // 绑定到指定端口

    // 绑定套接字到本地地址
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 开始监听连接请求
    if (listen(sockfd, 5) < 0)
    {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 循环监听并处理客户端连接
    while (1)
    {
        clilen = sizeof(cli_addr);                                      // 获取客户端地址结构的大小
        connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); // 接受客户端连接
        if (connfd < 0)
        {
            perror("Accept failed");
            continue;
        }

        // 创建子进程处理客户端请求
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Fork failed");
            close(connfd);
            continue;
        }
        else if (pid == 0)
        {
            // 子进程处理客户端请求
            close(sockfd);         // 关闭监听套接字
            handle_client(connfd); // 处理客户端请求
            exit(EXIT_SUCCESS);
        }
        else
        {
            // 父进程继续监听新的连接
            close(connfd); // 关闭连接套接字
        }
    }

    // 关闭监听套接字
    close(sockfd);
    return 0;
}
