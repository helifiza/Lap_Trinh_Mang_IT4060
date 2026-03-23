#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

int main (int argc, char* argv[]) {
    if(argc != 3) {
        printf("Su dung: sv_server <port> <log_file>\n");
        return 1;
    }

    int port = atoi(argv[1]);
    char* log_file = argv[2];

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(listen_socket, 5);

    printf("Server dang lang nghe tren port %d...\n", port);

    while(1){
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        SOCKET conn_sock = accept(listen_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        if(conn_sock != INVALID_SOCKET) {
            char recv_buff[256];
            int ret = recv(conn_sock, recv_buff, sizeof(recv_buff) - 1, 0);

            if(ret > 0) {
                recv_buff[ret] = '\0';

                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

                time_t t = time(NULL);
                struct tm* tm_info = localtime(&t);
                char time_str[20];
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

                char log_entry[512];
                sprintf(log_entry, "%s %s: %s\n",client_ip, time_str, recv_buff);

                printf("%s\n", log_entry);
                FILE *f = fopen(log_file, "a");
                if (f) {
                    fprintf(f, "%s", log_entry);
                    fclose(f);
                }
            }
            closesocket(conn_sock);
        }
    }
    closesocket(listen_socket);
    WSACleanup();
    return 0;
}