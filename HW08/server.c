#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>
#include "list.h"
#include "fileio.h"

#define BUFF_SIZE 1024

void handle_client(int client_sock, List **list);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PortNumber>\n", argv[0]);
        exit(1);
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    List *list = NULL;

    readFile(&list, "account.txt");

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error");
        exit(1);
    }

    listen(server_sock, 5);
    printf("Server listening on port %s\n", argv[1]);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("Accept error");
            continue;
        }

        if (fork() == 0) {
            close(server_sock);
            handle_client(client_sock, &list);
            exit(0);
        } else {
            close(client_sock);
        }
    }

    close(server_sock);
    return 0;
}

void handle_client(int client_sock, List **list) {
    char buff[BUFF_SIZE];
    int bytes_received;
    List *logged_user = NULL;
    int login_attempts = 0;

    while (1) {
        bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
        if (bytes_received <= 0) break;
        buff[bytes_received] = '\0';

        if (logged_user == NULL) {
            logged_user = find(*list, buff);
            if (logged_user == NULL || logged_user->ListUser.status == 0) {
                send(client_sock, "Invalid or blocked account\n", 28, 0);
                logged_user = NULL;
            } else {
                send(client_sock, "Password: ", 10, 0);
            }
        } else {
            if (strcmp(logged_user->ListUser.pass, buff) == 0) {
                send(client_sock, "Login successful\n", 18, 0);
                break;
            } else {
                login_attempts++;
                if (login_attempts >= 3) {
                    logged_user->ListUser.status = 0;
                    saveFile(*list, "account.txt");
                    send(client_sock, "Account blocked\n", 16, 0);
                    break;
                } else {
                    send(client_sock, "Incorrect password\n", 20, 0);
                }
            }
        }
    }

    while (1) {
        char buff2[BUFF_SIZE];
        int bytes_recv;
        bytes_recv = recv(client_sock, buff2, BUFF_SIZE - 1, 0);
        if (bytes_recv <= 0) break;
        buff2[bytes_recv] = '\0';
        printf("%s\n", buff2);
    }

    close(client_sock);
}
