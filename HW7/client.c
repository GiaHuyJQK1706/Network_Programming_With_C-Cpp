#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/utsname.h>  // Để lấy thông tin hệ thống
#include <fcntl.h>

#define BUFF_SIZE 1024

void send_file(int sock, const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("File open error");
        return;
    }

    char buffer[BUFF_SIZE];
    int bytes_read;
    while ((bytes_read = read(fd, buffer, BUFF_SIZE)) > 0) {
        send(sock, buffer, bytes_read, 0);
    }

    close(fd);
    printf("File sent successfully!\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP_ADDRESS> <PORT_NUMBER>\n", argv[0]);
        return 1;
    }

    int client_sock;
    struct sockaddr_in server_addr;
    char username[BUFF_SIZE], password[BUFF_SIZE], buff[BUFF_SIZE];

    // Step 1: Create TCP socket
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Step 2: Set up server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    // Step 3: Connect to the server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_sock);
        return 1;
    }

    printf("Connected to server\n");

    int login_attempts = 0;
    while (login_attempts < 3) {
        // Nhập username và gửi tới server
        printf("Enter username: ");
        fgets(buff, BUFF_SIZE, stdin);
        buff[strcspn(buff, "\n")] = '\0';  // Loại bỏ ký tự '\n'
        send(client_sock, buff, strlen(buff), 0);

        // Nhập password và gửi tới server
        printf("Enter password: ");
        fgets(buff, BUFF_SIZE, stdin);
        buff[strcspn(buff, "\n")] = '\0';  // Loại bỏ ký tự '\n'
        send(client_sock, buff, strlen(buff), 0);

        // Nhận phản hồi từ server về đăng nhập
        int bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Server closed connection or error occurred.\n");
            close(client_sock);
            return 1;
        }

        buff[bytes_received] = '\0';  // Đảm bảo chuỗi kết thúc bằng NULL

        if (strcmp(buff, "Account not exist!") == 0) {
            printf("%s\n", buff);
            close(client_sock);
            return 1;
        } else if (strcmp(buff, "Account has blocked!") == 0) {
            printf("%s\n", buff);
            close(client_sock);
            return 1;
        } else if (strcmp(buff, "Login successful") == 0) {
            printf("%s!\n");
            break;
        } else if (strncmp(buff, "Login failed", 12) == 0) {
            login_attempts++;
            printf("%s\n", buff);
            if (login_attempts >= 3) {
                printf("%s\n", buff);
                close(client_sock);
                return 1;
            }
        }
    }

    // Hiển thị menu chọn tác vụ
    printf("Choose an option (1: Send system info, 2: Send file): ");
    int option;
    scanf("%d", &option);
    getchar();

    send(client_sock, &option, sizeof(option), 0);

    if (option == 1) {
        // Send system info
        struct utsname sys_info;
        uname(&sys_info);

        snprintf(buff, BUFF_SIZE, "OS: %s, Node: %s, Release: %s, Version: %s, Machine: %s",
                 sys_info.sysname, sys_info.nodename, sys_info.release, sys_info.version, sys_info.machine);
        send(client_sock, buff, strlen(buff), 0);
    } else if (option == 2) {
        // Send file
        printf("Enter filename: ");
        fgets(buff, BUFF_SIZE, stdin);
        buff[strcspn(buff, "\n")] = 0;
        send_file(client_sock, buff);
    }

    close(client_sock);
    return 0;
}