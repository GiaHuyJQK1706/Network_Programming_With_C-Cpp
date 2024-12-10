#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "sha256.h"

#define BUFFER_SIZE 1024

void handle_string(const char *input, char *alpha, char *numeric) {
    int i = 0, a = 0, n = 0;
    while (input[i]) {
        if ((input[i] >= 'a' && input[i] <= 'z') || (input[i] >= 'A' && input[i] <= 'Z'))
            alpha[a++] = input[i];
        else if (input[i] >= '0' && input[i] <= '9')
            numeric[n++] = input[i];
        else {
            strcpy(alpha, "Invalid character in string.");
            numeric[0] = '\0';
            return;
        }
        i++;
    }
    alpha[a] = '\0';
    numeric[n] = '\0';
}

void process_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    char sha256_hex_buffer[SHA256_HEX_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0) break;
        if (strncmp(buffer, "FILE:", 5) == 0) {
            printf("Noi dung file da gui:\n%s\n", buffer + 5);
        } else {
            sha256_hex(buffer, strlen(buffer), sha256_hex_buffer);
            char alpha[SHA256_HEX_SIZE], numeric[SHA256_HEX_SIZE];
            handle_string(sha256_hex_buffer, alpha, numeric);
            printf("Chu cai: %s\n", alpha);
            printf("Chu so: %s\n", numeric);
            send(client_sock, alpha, strlen(alpha) + 1, 0);
        }
    }
    close(client_sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s PortNumber\n", argv[0]); //Nhap sai dau vao
        exit(1);
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket that bai!");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind that bai");
        close(server_sock);
        exit(1);
    }

    if (listen(server_sock, 5) < 0) {
        perror("Listen that bai!");
        close(server_sock);
        exit(1);
    }

    printf("Server listening on port %s...\n", argv[1]);

    while ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len)) >= 0) {
        process_client(client_sock);
    }

    close(server_sock);
    return 0;
}
