#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

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
                return 1; // Dang nhap thanh cong
            }
        }
    }
    fclose(fp);
    return 0; 
}

int main() 
{
    int server_fd, client_fds[MAX_CLIENTS], max_clients = MAX_CLIENTS;
    struct sockaddr_in server_addr, client_addr;
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds, activity, i, valread, sd, new_socket, addrlen;
    char buffer[BUFFER_SIZE];

    for(int i = 0; i < max_clients; i++) {
        client_fds[i] = 0;
    }
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Loi tao socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);
    if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Loi bind");
        exit(EXIT_FAILURE);
    }
    if(listen(server_fd, 3) < 0) {
        perror("Loi listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(client_addr);
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    while(1) {
        nfds = 1;
        for(i = 0; i < max_clients; i++) {
            if(client_fds[i] > 0) {
                fds[nfds].fd = client_fds[i];
                fds[nfds].events = POLLIN;
                nfds++;
            }
        }

        activity = poll(fds, nfds, -1);
        if((activity < 0) && (errno != EINTR)) {
            perror("Loi poll");
        }
        if(fds[0].revents & POLLIN) {
            if((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) < 0) {
                perror("Loi accept");
                exit(EXIT_FAILURE);
            }
            printf("New connection: socket fd is %d, ip is : %s, port : %d\n", new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            char *welcome_msg = "Welcome to the server. Please login with username and password\n";
            send(new_socket, welcome_msg, strlen(welcome_msg), 0);
            for(i = 0; i < max_clients; i++) {
                if(client_fds[i] == 0) {
                    client_fds[i] = new_socket;
                    break;
                }
        }
    }

    for(i = 1; i < nfds; i++)
    {
        if (fds[i].revents & POLLIN)
            {
                sd = fds[i].fd;

                memset(buffer, 0, BUFFER_SIZE);
                valread = recv(sd, buffer, BUFFER_SIZE, 0);

                if (valread <= 0)
                {
                    getpeername(sd,
                                (struct sockaddr *)&client_addr,
                                (socklen_t *)&addrlen);

                    printf("Host disconnected, ip %s, port %d\n",
                           inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port));

                    close(sd);

                    for (int j = 0; j < max_clients; j++)
                    {
                        if (client_fds[j] == sd)
                        {
                            client_fds[j] = 0;
                            break;
                        }
                    }
                }
                else
                {
                    if (strncmp(buffer, "userpass:", 9) == 0)
                    {
                        char *username = strtok(buffer + 9, " ");
                        char *password = strtok(NULL, "\n");
                        int authenticated = check_credentials(username, password);

                        if (authenticated)
                        {
                            char *success_message = "Login successful\n";
                            send(sd, success_message, strlen(success_message), 0);
                        }
                        else
                        {
                            char *error_message = "Invalid username or password\n";
                            send(sd, error_message, strlen(error_message), 0);
                        }
                    }
                    else
                    {
                        char command[BUFFER_SIZE];
                        snprintf(command, BUFFER_SIZE, "%.900s > out.txt", buffer);
                        system(command);

                        FILE *fp = fopen("out.txt", "r");

                        if (fp != NULL)
                        {
                            char result_buffer[BUFFER_SIZE];
                            memset(result_buffer, 0, BUFFER_SIZE);

                            fread(result_buffer,
                                  sizeof(char),
                                  BUFFER_SIZE - 1, fp);

                            send(sd, result_buffer,
                                 strlen(result_buffer), 0);

                            fseek(fp, 0, SEEK_END);
                            long size = ftell(fp);
                            if (size == 0)
                            {
                                char *error_message = "Error executing command\n";
                                send(sd, error_message,
                                     strlen(error_message), 0);
                            }

                            fclose(fp);
                        }
                    }
                }
            }
        }
    }
    return 0;
}