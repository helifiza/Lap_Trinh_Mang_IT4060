#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

// =======================================================
// CẤU HÌNH THÔNG TIN SINH VIÊN CỦA BẠN TẠI ĐÂY
#define MSSV        "20235362"  // Sửa thành MSSV của bạn
#define NGAY_SINH   01         // Sửa thành ngày sinh của bạn
// =======================================================

#define SERVER_HOST "lebavui.io.vn"
#define FTP_PORT    21
#define BUFFER_SIZE 8192

// Hàm gửi lệnh FTP và nhận phản hồi từ Control Channel (Port 21)
void send_ftp_cmd(int sock, const char *cmd, char *response) {
    char buf[BUFFER_SIZE];
    memset(buf, 0, sizeof(buf));
    
    if (cmd != NULL) {
        send(sock, cmd, strlen(cmd), 0);
        printf(">> %s", cmd); // In lệnh gửi đi (đã có sẵn \r\n)
    }
    
    int bytes_received = recv(sock, buf, sizeof(buf) - 1, 0);
    if (bytes_received > 0) {
        buf[bytes_received] = '\0';
        if (response) strcpy(response, buf);
        printf("<< %s", buf); // In phản hồi từ Server
    }
}

// Hàm kết nối tới một Server IP và Port bất kỳ
int connect_server(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        return -1;
    }
    return sock;
}

int main() {
    char cmd[256], res[BUFFER_SIZE];
    char username[50], password[50];
    
    // 1. Tạo Username và Password dựa trên thông tin cấu hình
    sprintf(username, "user_%s", MSSV);
    // Lấy 4 số cuối MSSV
    const char *last_4 = MSSV + (strlen(MSSV) - 4);
    sprintf(password, "%s%02d", last_4, NGAY_SINH);
    
    // Phân giải tên miền "lebavui.io.vn" sang IP
    struct hostent *he = gethostbyname(SERVER_HOST);
    if (!he) {
        perror("[-] Không phân giải được tên miền server");
        return 1;
    }
    char *server_ip = inet_ntoa(*(struct in_addr *)he->h_addr_list[0]);
    printf("[*] Server IP: %s\n", server_ip);

    // Kết nối tới Control Connection (Port 21)
    int ctrl_sock = connect_server(server_ip, FTP_PORT);
    if (ctrl_sock < 0) {
        perror("[-] Kết nối đến Port 21 thất bại");
        return 1;
    }
    send_ftp_cmd(ctrl_sock, NULL, res); // Nhận lời chào chào mừng (220)

    // Đăng nhập
    sprintf(cmd, "USER %s\r\n", username);
    send_ftp_cmd(ctrl_sock, cmd, res);
    
    sprintf(cmd, "PASS %s\r\n", password);
    send_ftp_cmd(ctrl_sock, cmd, res);
    
    if (strncmp(res, "230", 3) != 0) {
        printf("[-] Đăng nhập thất bại. Kiểm tra lại tài khoản/mật khẩu!\n");
        close(ctrl_sock);
        return 1;
    }

    // Chuyển chế độ truyền sang dạng Type I (Binary) để không lỗi file text
    send_ftp_cmd(ctrl_sock, "TYPE I\r\n", res);

    // 2. Chuyển sang chế độ PASV (Passive mode) để lấy danh sách file
    send_ftp_cmd(ctrl_sock, "PASV\r\n", res);
    
    // Tách IP và Port từ phản hồi của lệnh PASV: dạng "227 Entering Passive Mode (127,0,0,1,192,168)"
    int ip1, ip2, ip3, ip4, p1, p2;
    char *pasv_info = strchr(res, '(');
    sscanf(pasv_info, "(%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &p1, &p2);
    char data_ip[32];
    sprintf(data_ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    int data_port = p1 * 256 + p2; // Công thức tính Data Port

    // Mở kết nối dữ liệu (Data Connection)
    int data_sock = connect_server(data_ip, data_port);
    
    // Gửi lệnh LIST để lấy danh sách file
    send_ftp_cmd(ctrl_sock, "LIST\r\n", res);
    
    // Đọc danh sách file từ Data Socket
    char file_list[BUFFER_SIZE] = {0};
    int bytes = recv(data_sock, file_list, sizeof(file_list) - 1, 0);
    close(data_sock); // Đọc xong danh sách phải đóng ngay data socket này
    send_ftp_cmd(ctrl_sock, NULL, res); // Nhận phản hồi 226 Transfer complete

    // Tìm tên file dạng "question_xxxxxx.txt" trong danh sách
    char question_file[128] = {0};
    char *match = strstr(file_list, "question_");
    if (!match) {
        printf("[-] Không tìm thấy file câu hỏi nào.\n");
        close(ctrl_sock);
        return 1;
    }
    // Trích xuất chính xác tên file câu hỏi
    sscanf(match, "%s", question_file);
    // Bỏ ký tự thừa xuống dòng nếu có
    question_file[strcspn(question_file, "\r\n")] = 0;
    printf("[+] Tìm thấy file câu hỏi: %s\n", question_file);

    // 3. Tiến hành tải (download) nội dung file question
    send_ftp_cmd(ctrl_sock, "PASV\r\n", res); // Lại xin cổng PASV mới cho lượt truyền mới
    pasv_info = strchr(res, '(');
    sscanf(pasv_info, "(%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &p1, &p2);
    data_port = p1 * 256 + p2;
    
    data_sock = connect_server(data_ip, data_port);
    
    sprintf(cmd, "RETR %s\r\n", question_file);
    send_ftp_cmd(ctrl_sock, cmd, res);
    
    char question_content[512] = {0};
    bytes = recv(data_sock, question_content, sizeof(question_content) - 1, 0);
    question_content[bytes] = '\0';
    close(data_sock);
    send_ftp_cmd(ctrl_sock, NULL, res); // Nhận phản hồi 226 thành công
    
    printf("[+] Nội dung file tải về là:\n%s\n", question_content);

    // 4. Đảo ngược nội dung chuỗi (bỏ qua ký tự xuống dòng ở cuối nếu có)
    int len = strlen(question_content);
    while (len > 0 && (question_content[len-1] == '\n' || question_content[len-1] == '\r')) {
        question_content[len-1] = '\0';
        len--;
    }
    
    char answer_content[512] = {0};
    for (int i = 0; i < len; i++) {
        answer_content[i] = question_content[len - 1 - i];
    }
    answer_content[len] = '\0';
    printf("[+] Nội dung sau khi đảo ngược:\n%s\n", answer_content);

    // Tạo tên file answer tương ứng
    char answer_file[128];
    strcpy(answer_file, question_file);
    char *replace = strstr(answer_file, "question_");
    if (replace) {
        memcpy(replace, "answer___", 9); // Thay thế chữ "question_" bằng "answer___" bảo toàn độ dài
    }
    // Hoặc chuẩn hơn là tạo tên mới:
    sprintf(answer_file, "answer_%s", question_file + 9);

    // 5. Upload file answer lên server
    send_ftp_cmd(ctrl_sock, "PASV\r\n", res); // Xin cổng PASV mới để upload
    pasv_info = strchr(res, '(');
    sscanf(pasv_info, "(%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &p1, &p2);
    data_port = p1 * 256 + p2;
    
    data_sock = connect_server(data_ip, data_port);
    
    sprintf(cmd, "STOR %s\r\n", answer_file);
    send_ftp_cmd(ctrl_sock, cmd, res);
    
    // Gửi nội dung chuỗi đã đảo ngược qua kênh dữ liệu
    send(data_sock, answer_content, strlen(answer_content), 0);
    close(data_sock); // Đóng data channel để báo hiệu hết file
    send_ftp_cmd(ctrl_sock, NULL, res); // Nhận phản hồi hoàn tất từ server

    // Thoát an toàn
    send_ftp_cmd(ctrl_sock, "QUIT\r\n", res);
    close(ctrl_sock);
    
    printf("\n[+] ĐÃ HOÀN THÀNH BÀI TẬP XUẤT SẮC!\n");
    return 0;
}