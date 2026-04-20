#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

#define BUFFER_SIZE 1024

int main()
{
    int sock;
    char credentials[3000];
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    char loginSuccess[] = "Login successful\n";
    struct sockaddr_in server_addr;
    char server_ip[] = "127.0.0.1";
    int server_port = 9000;
    char buffer[BUFFER_SIZE];

    struct pollfd fds[2];
    int activity;
    ssize_t num_bytes;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Cấu hình poll
    fds[0].fd = sock;      // socket
    fds[0].events = POLLIN;

    fds[1].fd = STDIN_FILENO; // bàn phím
    fds[1].events = POLLIN;

    // Nhận welcome message
    num_bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (num_bytes > 0)
    {
        buffer[num_bytes] = '\0';
        printf("%s", buffer);
    }

    // ===== LOGIN =====
    while (1)
    {
        // Nhận yêu cầu từ server
        num_bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (num_bytes <= 0)
        {
            printf("Server disconnected\n");
            close(sock);
            return 0;
        }

        buffer[num_bytes] = '\0';
        printf("%s", buffer);   // In đúng nội dung server gửi

        // Nhập từ bàn phím
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        // Gửi lại cho server
        send(sock, buffer, strlen(buffer), 0);

        // Nếu server không gửi lại prompt nữa thì login thành công
        // Server của bạn chỉ lặp khi sai format
        // Nếu đúng format -> server không gửi lại prompt
        break;
    }

    // ===== COMMAND LOOP =====
    while (1)
    {
        activity = poll(fds, 2, -1);

        if (activity < 0)
        {
            perror("Poll error");
            break;
        }

        // Nếu nhập từ bàn phím
        if (fds[1].revents & POLLIN)
        {
            memset(buffer, 0, BUFFER_SIZE);
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = '\0';

            send(sock, buffer, strlen(buffer), 0);

            if (strcmp(buffer, "exit") == 0)
                break;
        }

        // Nếu có dữ liệu từ server
        if (fds[0].revents & POLLIN)
        {
            memset(buffer, 0, BUFFER_SIZE);
            num_bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);

            if (num_bytes <= 0)
            {
                printf("Server disconnected\n");
                break;
            }

            buffer[num_bytes] = '\0';
            printf("%s", buffer);
        }
    }

    close(sock);
    return 0;
}