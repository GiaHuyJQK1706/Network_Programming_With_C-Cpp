#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFSIZE 2048

void reverse_string_and_filter_alpha(char *str) {
    int len = strlen(str);
    char temp;
    int start = 0, end = len - 1;
    while (start < end) {
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
    char result[BUFSIZE];
    int j = 0;
    for (int i = 0; i < len; i++) {
        if (isalpha(str[i])) {
            result[j++] = str[i];
        }
    }
    result[j] = '\0';
    strcpy(str, result);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Use: %s PortNumber\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    int server_sock;
    struct sockaddr_in server_addr, client1_addr;
    char buffer[BUFSIZE];
    socklen_t client_len = sizeof(client1_addr);

    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket create failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    while(1) {
        recvfrom(server_sock, buffer, BUFSIZE, 0, (struct sockaddr *)&client1_addr, &client_len);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "@") == 0 || strcmp(buffer, "#") == 0) {
            break;
        }

        int valid = 1;
        for (int i = 0; i < strlen(buffer); i++) {
            if (!isalnum(buffer[i])) {
                valid = 0;
                break;
            }
        }
        if (!valid) {
            printf("Error \n");
            strcpy(buffer, "Error. String contains invalid characters");
        } else {
            reverse_string_and_filter_alpha(buffer);
        }   

        recvfrom(server_sock, NULL, 0, 0, (struct sockaddr *)&client1_addr, &client_len);
        sendto(server_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&client1_addr, client_len);
    }

    close(server_sock);
    return 0;
}