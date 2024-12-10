#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "list.h"
#include "fileio.h"

#define BUFF_SIZE 16384

void save_to_csv(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "a");
    if (fp == NULL) {
        perror("File open error");
        return;
    }
    fprintf(fp, "%s\n", content);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PORT_NUMBER>\n", argv[0]);
        return 1;
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    char buff[BUFF_SIZE];

    List *list = NULL;
    readFile(&list, "account.txt");

    // Tạo socket TCP
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Thiết lập địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        return 1;
    }

    listen(server_sock, 5);
    printf("Server listening on port %s\n", argv[1]);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &sin_size);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected\n");

        int bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
        buff[bytes_received] = '\0';  // Đảm bảo chuỗi kết thúc bằng NULL
        char username[BUFF_SIZE];
        strcpy(username, buff);

        bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
        buff[bytes_received] = '\0';
        char password[BUFF_SIZE];
        strcpy(password, buff);

        int login_attempts = 0;

        List *user = find(list, username);
        if (!user) {
            send(client_sock, "Account not exist!", 18, 0);
            close(client_sock);
            return 1;
        }

        if (user->ListUser.status == 0) {
            send(client_sock, "Account has blocked!", 20, 0);
            close(client_sock);
            return 1;
        }

        while (login_attempts < 3) {
            if (strcmp(user->ListUser.pass, password) == 0) {
                send(client_sock, "Login successful", 16, 0);
                break;
            } else {
                login_attempts++;
                if (login_attempts >= 3) {
                    user->ListUser.status = 0;
                    send(client_sock, "Login failed! Account blocked!", 30, 0);
                    break;
                } else {
                    send(client_sock, "Login failed, try again.", 25, 0);
                    bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
                    buff[bytes_received] = '\0';
                    strcpy(password, buff);
                }
            }
        }

        ssize_t total_bytes_recv = 0;
        
        int option;
        bytes_received = recv(client_sock, &option, sizeof(option), 0);
        if (option == 1) {
            int flog = open("infodevice.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
            bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
            total_bytes_recv += bytes_received;
            write(flog, buff, bytes_received);
            close(flog);
        }
        if (option == 2) {
            int fd = open("recv_file_10mb.csv", O_WRONLY | O_CREAT | O_APPEND, 0644);
            while (1) {
                bytes_received = recv(client_sock, buff, BUFF_SIZE, 0);
                if (bytes_received < 0) {
                    perror("Receive error");
                    break;
                } else if (bytes_received == 0) {
                    break;
                }
                total_bytes_recv += bytes_received;
                write(fd, buff, bytes_received);
            }
            close(fd);
        }
        
        printf("Total bytes received: %zd MB %zd KB %zd B\n", 
                total_bytes_recv/1048576,
                total_bytes_recv/1024%1024,
                total_bytes_recv%1024);
        printf("File received and saved\n");
        close(client_sock);
    }
    close(server_sock);
    return 0;
}