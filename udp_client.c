#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include<unistd.h>

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char * argv[]){
    if (argc < 3) {
        printf("Cach dung: %s <IP> <Port>\n", argv[0]);
        return 1;
    }

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(client ==-1){
        perror("sock() failed");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    char buf[256];
    printf("UDP Client da bat dau. Chon 'exit' de thoat.\n");
    while(1){
        printf("Me: ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }
        buf[strcspn(buf,"\n")]= 0;
        sendto(client, buf, strlen(buf), 0,(struct sockaddr*)&addr, sizeof(addr));
        if (strcmp(buf, "exit") == 0) break;

        int ret = recvfrom(client, buf, sizeof(buf)-1,0, NULL, NULL);
        if(ret<=0){
            break;
        }
        buf[ret] = '\0';
        printf("You: %s\n", buf);
    }
    closesocket(client);
    WSACleanup();
    return 0;
}
