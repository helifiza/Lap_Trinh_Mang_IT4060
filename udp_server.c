#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include<unistd.h>


#pragma comment(lib, "ws2_32.lib")

int main(int argc, char *argv[]){
    if (argc < 2) {
        printf("Cach dung: %s <Port>\n", argv[0]);
        return 1;
    }
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server == INVALID_SOCKET) {
        printf("socket() failed: %d\n", WSAGetLastError());
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        printf("bind() failed: %d\n", WSAGetLastError());
        closesocket(server);
        WSACleanup();
        return 1;
    }
    printf("UDP Server dang lang nghe tren cong %s...\n", argv[1]);


    char buf[256];
    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);

    while(1){
        int ret = recvfrom(server, buf, sizeof(buf)-1, 0,(struct sockaddr *)&client_addr, &client_len);
        if(ret<=0){
            break;
        }
        buf[ret] = '\0';
        printf("You: %s\n", buf);

        printf("Me: ");
        fgets(buf, sizeof(buf), stdin);
        sendto(server, buf, strlen(buf)-1, 0, (struct sockaddr *)&client_addr, client_len);
        printf("\n");
    }
    closesocket(server);
    WSACleanup();
    return 0;
}