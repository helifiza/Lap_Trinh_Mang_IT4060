#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define PORT 8888

int check_credentials(char *username, char *password) {
    FILE *fp;
    char line[BUFFER_SIZE];
    char *token;

    fp = fopen("credentials.txt", "r");
    if (fp == NULL) {
        perror("Loi mo file credentials.txt");
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        token = strtok(line, ":");
        if (token != NULL && strcmp(token, username) == 0) {
            token = strtok(NULL, "\n");
            if (token != NULL && strcmp(token, password) == 0) {
                fclose(fp);
                return 1;
            }
        }
    }

    fclose(fp);
    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];

    char *welcome_msg = "Welcome to the server. Please login using:username password\n";
    send(client_socket, welcome_msg, strlen(welcome_msg), 0);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (valread <= 0) {
            printf("Client disconnected\n");
            break;
        }
        char *username = strtok(buffer, " ");
        char *password = strtok(NULL, "\n");
        
        if (username == NULL || password == NULL) {
            char *error = "Invalid format\n";
            send(client_socket, error, strlen(error), 0);
            continue;
        }

        int authenticated = check_credentials(username, password);

        if (authenticated) {
            char *success = "Login successful\n";
            send(client_socket, success, strlen(success), 0);
        } else {
            char *error = "Invalid username or password\n";
            send(client_socket, error, strlen(error), 0);
        }
    }

    close(client_socket);
    exit(0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Loi tao socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Loi bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Loi listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (new_socket < 0) {
            perror("Loi accept");
            continue;
        }

        printf("New connection from %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            close(new_socket);
        }
        else if (pid == 0) {
            // Process con
            close(server_fd);
            handle_client(new_socket);
        }
        else {
            close(new_socket);

            while (waitpid(-1, NULL, WNOHANG) > 0);
        }
    }

    close(server_fd);
    return 0;
}