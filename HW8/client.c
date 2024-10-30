#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024

int attmp = 1;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
        exit(1);
    }

    int client_sock;
    struct sockaddr_in server_addr;
    char buff[BUFF_SIZE];
    char usn[BUFF_SIZE];
    int bytes_received;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect error");
        exit(1);
    }

    while (1) {
        printf("Enter message: ");
        fgets(buff, BUFF_SIZE, stdin);
        buff[strcspn(buff, "\n")] = '\0';
        if (attmp == 1) { strcpy(usn, buff); }
        
        attmp++;
        send(client_sock, buff, strlen(buff), 0);
        bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
        if (bytes_received <= 0) break;

        buff[bytes_received] = '\0';
        printf("%s\n", buff);

        if (strcmp(buff, "Invalid or blocked account\n") == 0) {attmp--;};
        if (strcmp(buff, "Login successful\n") == 0) break;
    }

    while (1) {
        printf("Do you want to Logout? (Yes/No)?: ");
        fgets(buff, BUFF_SIZE, stdin);
        buff[strcspn(buff, "\n")] = '\0';
        if (strcmp(buff, "Yes") == 0) {
            char buff2[BUFF_SIZE];
            char usn_tmp[BUFF_SIZE];
            strcpy(usn_tmp, usn);
            strcpy(buff2, strcat(usn_tmp, " logout"));
            send(client_sock, buff2, strlen(buff2), 0);
            break;
        } else if (strcmp(buff, "No") == 0) {
            char buff2[BUFF_SIZE];
            char usn_tmp[BUFF_SIZE];
            strcpy(usn_tmp, usn);
            strcpy(buff2, strcat(usn_tmp, " continue"));
            send(client_sock, buff2, strlen(buff2), 0);
        } else if (strcmp(buff, "No") != 0 && strcmp(buff, "Yes") != 0) {
            char buff2[BUFF_SIZE];
            char usn_tmp[BUFF_SIZE];
            strcpy(usn_tmp, usn);
            strcpy(buff2, strcat(usn_tmp, " wrong cmd"));
            send(client_sock, buff2, strlen(buff2), 0);
        }
    }

    close(client_sock);
    return 0;
}
