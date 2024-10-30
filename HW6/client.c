#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void send_string(int client_sock) {
    char input[BUFFER_SIZE];
    while (1) {
        printf("Nhap 1 chuoi bat ky (Chuoi rong se thoat): ");
        fgets(input, BUFFER_SIZE, stdin);
        if (input[0] == '\n') break;
        send(client_sock, input, strlen(input), 0);
        char response[BUFFER_SIZE];
        recv(client_sock, response, BUFFER_SIZE, 0);
        printf("Server response: %s\n", response);
    }
}

void send_file(int client_sock) {
    char filename[BUFFER_SIZE];
    printf("Nhap filename: ");
    scanf("%s", filename);
    FILE *file = fopen(filename, "r+");
    if (!file) {
        perror("File open failed");
        return;
    }
    char buffer[BUFFER_SIZE] = "FILE:";
    fread(buffer + 5, 1, BUFFER_SIZE - 5, file);
    fclose(file);
    send(client_sock, buffer, strlen(buffer), 0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s IPAddress PortNumber\n", argv[0]);
        exit(1);
    }

    int client_sock;
    struct sockaddr_in server_addr;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Tao socket that bai!");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_sock);
        exit(1);
    }
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection that bai!");
        close(client_sock);
        exit(1);
    }

    printf("MENU\n1. Gui xau bat ky\n2. Gui noi dung mot file\n");

    int choice;
    scanf("%d", &choice);
    getchar(); 
    if (choice == 1) {
        send_string(client_sock);
    } else if (choice == 2) {
        send_file(client_sock);
    }
    close(client_sock);
    return 0;
}
