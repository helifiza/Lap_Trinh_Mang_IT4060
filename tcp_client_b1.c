#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <string.h>   

#pragma comment(lib, "ws2_32.lib")

#define LARGE_BUFF_SIZE 2048

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Su dung: tcp_client <dia chi IP> <cong>\n");
        return 1;
    }

    char* serverIp = argv[1];
    int serverPort = atoi(argv[2]);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Khoi tao Winsock that bai\n");
        return 1;
    }

    SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET) {
        printf("Loi %d: Khong the tao socket\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverIp, &serverAddr.sin_addr) <= 0) {
        printf("Loi: Dia chi IP khong hop le\n");
        closesocket(client);
        WSACleanup();
        return 1;
    }

    if (connect(client, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Loi %d: Khong the ket noi den server %s:%d\n", WSAGetLastError(), serverIp, serverPort);
        closesocket(client);
        WSACleanup();
        return 1;
    }

    printf("Da ket noi den server %s:%d\n", serverIp, serverPort);
    printf("Nhap du lieu de gui (Nhan Enter khong nhap gi de thoat):\n");

    char buff[LARGE_BUFF_SIZE]; 
    while (1) {
        printf("> ");
        
        if (fgets(buff, sizeof(buff), stdin) == NULL) {
            break;
        }

        buff[strcspn(buff, "\n")] = 0; 

        if (strlen(buff) == 0) {
            break;
        }

        int ret = send(client, buff, (int)strlen(buff), 0);
        if (ret == SOCKET_ERROR) {
            printf("Loi %d: Khong the gui du lieu\n", WSAGetLastError());
            break;
        }
        printf("Da gui %d byte den server\n", ret);
    }

    closesocket(client);
    WSACleanup();
    printf("Da ngat ket noi den server\n");
    return 0;
}