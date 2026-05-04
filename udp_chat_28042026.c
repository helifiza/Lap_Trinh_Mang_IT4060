#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: %s <port> <remote_ip> <remote_port>\n", argv[0]);
        return 1;
    }

    int local_port = atoi(argv[1]);
    char *remote_ip = argv[2];
    int remote_port = atoi(argv[3]);

    int sockfd;
    struct sockaddr_in local_addr, remote_addr;
    socklen_t addr_len = sizeof(remote_addr);
    char buffer[BUFFER_SIZE];

    fd_set readfds;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket failed");
        return 1;
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(local_port);

    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    inet_pton(AF_INET, remote_ip, &remote_addr.sin_addr);

    printf("UDP Chat started...\n");
    printf("Local port: %d\n", local_port);
    printf("Remote: %s:%d\n", remote_ip, remote_port);

    while (1) {

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);      
        FD_SET(STDIN_FILENO, &readfds); 

        int max_fd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Select error");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, BUFFER_SIZE, stdin);

            sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&remote_addr, addr_len);
        }

        if (FD_ISSET(sockfd, &readfds)) {
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr *)&remote_addr, &addr_len);

            if (n > 0) {
                buffer[n] = '\0';
                printf("Friend: %s", buffer);
            }
        }
    }

    close(sockfd);
    return 0;
}