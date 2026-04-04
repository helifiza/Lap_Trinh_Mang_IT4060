#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 64

#define WAIT_NAME 0
#define WAIT_MSSV 1

typedef struct {
    SOCKET fd;
    int state;
    char name[100];
    char mssv[20];
} Client;

void generate_email(char *name, char *mssv, char *email) {
    char temp[200];
    strcpy(temp, name);

    char *words[20];
    int count = 0;

    char *token = strtok(temp, " ");
    while (token != NULL) {
        words[count++] = token;
        token = strtok(NULL, " ");
    }

    if (count < 2) {
        sprintf(email, "invalid_name@sis.hust.edu.vn");
        return;
    }

    char *ten = words[count - 1];
    char *mssv_cut = mssv + 2;

    // chuyển tên thành chữ thường
    for (int i = 0; ten[i]; i++)
        ten[i] = tolower(ten[i]);
    char initials[50] = "";

    for (int i = 0; i < count - 1; i++) {
        char c = tolower(words[i][0]);
        int len = strlen(initials);
        initials[len] = c;
        initials[len + 1] = '\0';
    }

    sprintf(email, "%s.%s%s@sis.hust.edu.vn",
            ten,
            initials,
            mssv_cut);
}

int main() {

    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    u_long mode = 1;
    ioctlsocket(listener, FIONBIO, &mode);

    BOOL opt = TRUE;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Server listening on port 8080...\n");

    Client clients[MAX_CLIENTS];
    int nclients = 0;

    char buf[256];

    while (1) {

        // Accept client
        SOCKET client = accept(listener, NULL, NULL);

        if (client != INVALID_SOCKET) {
            printf("New client connected\n");

            ioctlsocket(client, FIONBIO, &mode);

            clients[nclients].fd = client;
            clients[nclients].state = WAIT_NAME;
            memset(clients[nclients].name, 0, sizeof(clients[nclients].name));
            memset(clients[nclients].mssv, 0, sizeof(clients[nclients].mssv));

            send(client, "Nhap ho ten: ", 14, 0);

            nclients++;
        }

        // Xử lý client
        for (int i = 0; i < nclients; i++) {

            int len = recv(clients[i].fd, buf, sizeof(buf) - 1, 0);

            if (len > 0) {

                buf[len] = 0;
                buf[strcspn(buf, "\r\n")] = 0;

                if (clients[i].state == WAIT_NAME) {

                    strcpy(clients[i].name, buf);
                    send(clients[i].fd, "Nhap MSSV: ", 12, 0);
                    clients[i].state = WAIT_MSSV;
                }
                else if (clients[i].state == WAIT_MSSV) {

                    strcpy(clients[i].mssv, buf);

                    char email[150];
                    generate_email(clients[i].name, clients[i].mssv, email);

                    char msg[200];
                    sprintf(msg, "Email cua ban: %s\n", email);

                    send(clients[i].fd, msg, strlen(msg), 0);

                    clients[i].state = WAIT_NAME;
                    send(clients[i].fd, "Nhap ho ten: ", 14, 0);
                }
            }
            else if (len == SOCKET_ERROR) {

                int err = WSAGetLastError();

                if (err == WSAEWOULDBLOCK) {
                    continue;
                }
                else {
                    printf("Client disconnected\n");
                    closesocket(clients[i].fd);
                }
            }
        }
    }

    closesocket(listener);
    WSACleanup();
    return 0;
}

