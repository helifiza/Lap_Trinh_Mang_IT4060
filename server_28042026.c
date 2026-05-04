#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 9000
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

void ma_hoa(char *str) {
    for(int i = 0; str[i] !='\0';i++){
        if(str[i] >= 'A'  && str[i] <= 'Y') str[i]++;
        else if(str[i] == 'Z') str[i] = 'A';
        else if(str[i] >= 'a'  && str[i] <= 'y') str[i]++;
        else if(str[i] == 'z') str[i] = 'a';
        else if(str[i] >= '0' && str[i] <='9') str[i] = '9' -  str[i] + '0';
        
    }
}

int main() {
    int server_fd, new_socket, client_socket[MAX_CLIENTS];
    int max_sd, activity, sd;
    int client_count = 0;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    fd_set readfds;

    for (int i = 0; i < MAX_CLIENTS; i++)
        client_socket[i] = 0;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Server dang lang nghe tren port %d...\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (sd > 0)
                FD_SET(sd, &readfds);

            if (sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Select error");
            continue;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd,
                                     (struct sockaddr *)&address,
                                     (socklen_t *)&addrlen)) < 0) {
                perror("Accept");
                exit(EXIT_FAILURE);
            }

            printf("Client moi ket noi\n");

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    client_count++;
                    break;
                }
            }

            char welcome[100];
            sprintf(welcome, "Xin chao. Hien co %d clients dang ket noi.\n",
                    client_count);
            send(new_socket, welcome, strlen(welcome), 0);
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE - 1);

                if (valread <= 0) {
                    close(sd);
                    client_socket[i] = 0;
                    client_count--;
                    printf("Client ngat ket noi\n");
                } else {
                    buffer[valread] = '\0';

                    buffer[strcspn(buffer, "\r\n")] = 0;

                    if (strcmp(buffer, "exit") == 0) {
                        char bye[] = "Tam biet!\n";
                        send(sd, bye, strlen(bye), 0);
                        close(sd);
                        client_socket[i] = 0;
                        client_count--;
                        printf("Client thoat\n");
                    } else {
                        ma_hoa(buffer);
                        send(sd, buffer, strlen(buffer), 0);
                    }
                }
            }
        }
    }

    return 0;
}