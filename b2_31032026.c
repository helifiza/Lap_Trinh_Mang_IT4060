#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    
    if(argc != 4){
        printf("Nhap sai cu phap!");
        return 1;
    }

    int port_s = atoi(argv[1]);
    char *ip_d = argv[2];
    int port_d = atoi(argv[3]);

    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in local_addr, dest_addr;
    char buffer[BUFFER_SIZE];

    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup loi\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == INVALID_SOCKET) {
        printf("Tao socket loi\n");
        WSACleanup();
        return 1;
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(port_s);

    if(bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR) {
        printf("Bind loi\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port_d);
    inet_pton(AF_INET, ip_d, &dest_addr.sin_addr);

    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
    printf("Dang lang nghe tren port %d...\n", port_s);
    printf("Dang gui tin nhan den %s:%d...\n", ip_d, port_d);
    while(1){
        struct sockaddr_in sender_addr;
        int sender_len = sizeof(sender_addr);
        int recv_len = recvfrom(sock, buffer, BUFFER_SIZE-1, 0, (struct sockaddr *)&sender_addr, &sender_len);
        if(recv_len > 0) {
            buffer[recv_len] = '\0';
            printf("da nhan tin nhan: %s\n", buffer);
        } 
        else if(recv_len == SOCKET_ERROR) {
            if(WSAGetLastError() != WSAEWOULDBLOCK) {
                printf("Loi recvfrom\n");
                break;
            }
        }
        if(_kbhit()){
            fgets(buffer, BUFFER_SIZE, stdin);
            sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if(strcmp(buffer, "exit\n") == 0) break;
        }
        Sleep(1);
    }
    closesocket(sock);
    WSACleanup();
    return 0;
}