#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 9000
#define MAX_CLIENTS 10
#define MAX_TOPICS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket;
    char topics[MAX_TOPICS][50];
    int topic_count;
} Client;

Client clients[MAX_CLIENTS];

void add_topic(Client *c, char *topic) {
    for(int i=0;i<c->topic_count;i++)
        if(strcmp(c->topics[i], topic)==0)
            return;

    if(c->topic_count < MAX_TOPICS) {
        strcpy(c->topics[c->topic_count++], topic);
    }
}

void remove_topic(Client *c, char *topic) {
    for(int i=0;i<c->topic_count;i++) {
        if(strcmp(c->topics[i], topic)==0) {
            for(int j=i;j<c->topic_count-1;j++)
                strcpy(c->topics[j], c->topics[j+1]);
            c->topic_count--;
            break;
        }
    }
}

int subscribed(Client *c, char *topic) {
    for(int i=0;i<c->topic_count;i++)
        if(strcmp(c->topics[i], topic)==0)
            return 1;
    return 0;
}

int main() {
    int server_fd, new_socket, max_sd, activity, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    fd_set readfds;

    for(int i=0;i<MAX_CLIENTS;i++) {
        clients[i].socket = 0;
        clients[i].topic_count = 0;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server dang hoat dong tren port %d...\n", PORT);

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for(int i=0;i<MAX_CLIENTS;i++) {
            int sd = clients[i].socket;
            if(sd > 0)
                FD_SET(sd, &readfds);
            if(sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd,
                                (struct sockaddr *)&address,
                                (socklen_t*)&addrlen);

            for(int i=0;i<MAX_CLIENTS;i++) {
                if(clients[i].socket == 0) {
                    clients[i].socket = new_socket;
                    clients[i].topic_count = 0;
                    break;
                }
            }
            printf("Co mot client moi ket noi\n");
        }

        for(int i=0;i<MAX_CLIENTS;i++) {
            int sd = clients[i].socket;

            if(sd > 0 && FD_ISSET(sd, &readfds)) {

                valread = recv(sd, buffer, BUFFER_SIZE, 0);

                if(valread == 0) {
                    close(sd);
                    clients[i].socket = 0;
                    printf("Client da ngat ket noi\n");
                }
                else {
                    buffer[valread] = 0;

                    char *cmd = strtok(buffer, " ");

                    if(strcmp(cmd, "SUB") == 0) {
                        char *topic = strtok(NULL, "\n");
                        add_topic(&clients[i], topic);
                        printf("Client SUB %s\n", topic);
                    }

                    else if(strcmp(cmd, "UNSUB") == 0) {
                        char *topic = strtok(NULL, "\n");
                        remove_topic(&clients[i], topic);
                        printf("Client UNSUB %s\n", topic);
                    }

                    else if(strcmp(cmd, "PUB") == 0) {
                        char *topic = strtok(NULL, " ");
                        char *msg = strtok(NULL, "\n");

                        char sendbuf[BUFFER_SIZE];
                        snprintf(sendbuf, BUFFER_SIZE,
                                 "[%s]: %s\n", topic, msg);

                        for(int j=0;j<MAX_CLIENTS;j++) {
                            if(clients[j].socket > 0 &&
                               subscribed(&clients[j], topic)) {
                                send(clients[j].socket,
                                     sendbuf,
                                     strlen(sendbuf),
                                     0);
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}