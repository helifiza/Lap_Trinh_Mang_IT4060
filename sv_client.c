#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <string.h>   

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996) 

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Su dung: sv_client <IP server> <port>\n");
        return 1;
    }

    char* server_ip = argv[1];
    int port = atoi(argv[2]);

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Loi khoi tao Winsock. Loi: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        printf("Loi tao socket. Loi: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Loi ket noi den server. Loi: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    char mssv[10], hoten[200], ngaysinh[14], diem[4];
    printf("Nhap MSSV: ");
    fgets(mssv, sizeof(mssv), stdin);
    mssv[strcspn(mssv, "\n")] = 0;
    printf("Nhap Ho Ten: ");
    fgets(hoten, sizeof(hoten), stdin);
    hoten[strcspn(hoten, "\n")] = 0;
    printf("Nhap Ngay Sinh (dd/mm/yyyy): ");
    fgets(ngaysinh, sizeof(ngaysinh), stdin);
    ngaysinh[strcspn(ngaysinh, "\n")] = 0;
    printf("Nhap Diem: ");
    fgets(diem, sizeof(diem), stdin);
    diem[strcspn(diem, "\n")] = 0;

    char send_buffer[256];
    sprintf(send_buffer, "%s %s %s %s", mssv, hoten, ngaysinh, diem);

    send(client_socket, send_buffer, strlen(send_buffer), 0);
    printf("Da gui du lieu den server: %s\n", send_buffer);

    closesocket(client_socket);
    WSACleanup();
    return 0;

}