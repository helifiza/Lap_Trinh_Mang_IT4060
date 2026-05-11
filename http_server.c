#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define PORT 8080
#define WORKERS 5
#define BUFFER_SIZE 1024

void handle_client(int client)
{
    char buf[BUFFER_SIZE];
    int ret = recv(client, buf, sizeof(buf) - 1, 0);

    if (ret <= 0) {
        close(client);
        return;
    }

    buf[ret] = 0;
    printf("PID %d received request:\n%s\n", getpid(), buf);

    char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>Chao mung ban den voi Bai tap ltm cua Thanh Thuy</h1></body></html>";

    send(client, response, strlen(response), 0);
    close(client);
}

int main()
{
    int listener;
    struct sockaddr_in server_addr;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket error");
        exit(1);
    }

    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        exit(1);
    }

    if (listen(listener, 10) < 0) {
        perror("listen error");
        exit(1);
    }

    printf("Server dang chay tren port %d\n", PORT);

    // Prefork worker processes
    for (int i = 0; i < WORKERS; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            // Worker process
            while (1) {
                int client = accept(listener, NULL, NULL);
                if (client < 0) {
                    perror("accept error");
                    continue;
                }

                handle_client(client);
            }
            exit(0);
        }
    }

    // Parent waits
    while (wait(NULL) > 0);

    close(listener);
    return 0;
}