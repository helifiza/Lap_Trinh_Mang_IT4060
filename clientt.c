#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main() {

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Socket failed\n");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Connect failed\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to server!\n");

    char buffer[512];
    int len;

    while (1) {

        // Nhận yêu cầu từ server
        len = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) break;

        buffer[len] = 0;
        printf("%s", buffer);

        // Nhập dữ liệu từ bàn phím
        char input[256];
        fgets(input, sizeof(input), stdin);

        send(sock, input, strlen(input), 0);

        // Nếu vừa nhập MSSV thì server sẽ gửi email
        if (strstr(buffer, "Nhap MSSV") != NULL) {

            len = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (len <= 0) break;

            buffer[len] = 0;
            printf("%s", buffer);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}