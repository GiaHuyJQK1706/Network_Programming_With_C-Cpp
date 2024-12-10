#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFSIZE 2048

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Using: %s IPAddress PortNumber\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket create failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("IP address not valid");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Enter string: ");
        fgets(buffer, BUFSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "@") == 0 || strcmp(buffer, "#") == 0) {
            break;
        }

        sendto(client_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    
        int n = recvfrom(client_sock, buffer, BUFSIZE, 0, NULL, NULL);
        if (n > 0){
            buffer[n] = '\0';
            printf("Result: %s\n", buffer);
        } else {
            continue;
        }
    }

    close(client_sock);
    return 0;
}