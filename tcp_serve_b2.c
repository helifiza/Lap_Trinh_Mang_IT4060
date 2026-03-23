#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <string.h>   

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996) 

#define LARGE_BUFF_SIZE 2048

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Su dung: tcp_server <cong> <tep_tin_chao> <tep_tin_luu>\n");
        return 1;
    }

    int port = atoi(argv[1]);
    char *greetingFile = argv[2];
    char *outputFile = argv[3];

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Loi khoi tao Winsock\n");
        return 1;
    }

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        printf("Loi tao socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serverAddr.sin_port = htons(port);

    if (bind(listenSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Loi bind: %d\n", WSAGetLastError());
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    if (listen(listenSock, 5) == SOCKET_ERROR) {
        printf("Loi listen: %d\n", WSAGetLastError());
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    printf("Server dang lang nghe tren cong %d...\n", port);

    while (1) {
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET connSock = accept(listenSock, (struct sockaddr *)&clientAddr, &clientAddrLen);
        
        if (connSock != INVALID_SOCKET) {
            printf("Loi accept: %d\n", WSAGetLastError());
            continue;
        }

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
        printf("Co ket noi moi tu: %s:%d\n", clientIP, ntohs(clientAddr.sin_port));

        FILE *fGreeting = fopen(greetingFile, "rb");
        if (fGreeting != NULL) {
            char sendBuff[LARGE_BUFF_SIZE];
            int bytesRead;
            while ((bytesRead = fread(sendBuff, 1, sizeof(sendBuff), fGreeting)) > 0) {
                send(connSock, sendBuff, bytesRead, 0);
            }
            fclose(fGreeting);
            printf(" Da gui cau chao cho client.\n");
        } else {
            printf(" Khong the mo tep tin cau chao: %s\n", greetingFile);
        }

        FILE *fOutput = fopen(outputFile, "ab"); 
        if (fOutput != NULL) {
            char recvBuff[LARGE_BUFF_SIZE];
            int ret;
            printf(" Dang nhan du lieu tu client va luu vao %s...\n", outputFile);
            
            while ((ret = recv(connSock, recvBuff, sizeof(recvBuff), 0)) > 0) {
                fwrite(recvBuff, 1, ret, fOutput);
            }
            
            fclose(fOutput);
            if (ret == 0) {
                printf(" Client da ngat ket noi.\n");
            } else {
                printf(" Loi khi nhan du lieu.\n");
            }
        } else {
            printf(" Khong the mo tep tin luu tru: %s\n", outputFile);
        }

        closesocket(connSock);
        printf("--- Cho ket noi tiep theo ---\n");
    }


    closesocket(listenSock);
    WSACleanup();
    return 0;
}
