/*UDP Echo Server*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include "list.h"
#include "fileio.h"

#define BUFF_SIZE 1024

void sha256_hash(const char *data, char *digest) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data, strlen(data));
    SHA256_Final((unsigned char *)digest, &sha256);
}

void encode(const char *src, char *des) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    sha256_hash(src, (char *)digest);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&des[i * 2], "%02x", digest[i]);
    }
    des[2 * SHA256_DIGEST_LENGTH] = '\0';
}

int split_alpha_num(const char *str, char *alpha, char *num) {
    int n = strlen(str);
    int j, k;
    j = k = 0;
    for (int i = 0; i < n; i++) {
        if (isalpha(str[i])) {
            alpha[j++] = str[i];
        } else if (isdigit(str[i])) {
            num[k++] = str[i];
        } else {
            return 0;
        }
    }

    alpha[j] = '\0';
    num[k] = '\0';
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Wrong arguments!\nCorrect usage: %s <PORT_NUMBER>\n", argv[0]);
        return 1;
    }

    List *list = NULL;
	List *log = NULL;
    int loginCount = 0;
    bool userLoginCorrect = false;
    bool passwordLoginCorrect = false;
	
	readFile(&list, "account.txt");

    const int PORT = atoi(argv[1]);
    int server_sock; /* file descriptors */
    char buff[BUFF_SIZE];
    int bytes_sent, bytes_received;
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    int n_client __attribute__((unused)) = 0;
    socklen_t sin_size ;

    //Step 1: Construct a UDP socket
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {  /* calls socket() */
        perror("\nError: ");
        exit(0);
    }

    //Step 2: Bind address to socket
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);   /* Remember htons() from "Conversions" section? =) */
    server.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY puts your IP address automatically */
    bzero(&(server.sin_zero), 8); /* zero the rest of the structure */

    if (bind(server_sock, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) { /* calls bind() */
        perror("\nError: ");
        exit(0);
    }
    printf("Server listening on port %d\n", PORT);

    //Step 3: Communicate with clients
    while (1) {
        struct sockaddr *clients[1];
        sin_size = sizeof(struct sockaddr_in);

        char alpha[BUFF_SIZE / 2];
        char num[BUFF_SIZE / 2];
        // Receive from client 1
        bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &client, &sin_size);
        clients[0] = (struct sockaddr *) &client;
        printf("%d\n", client.sin_port);

        if (bytes_received < 0)
            perror("\nError: ");
        else {
            buff[bytes_received] = '\0';
            printf("Received from client 1\n");
            printf("%s\n", buff);
            if (log!=NULL && passwordLoginCorrect){
                if (strcmp(buff,"bye")==0){
                    char message[BUFF_SIZE] = "Goodbye ";
                    strcat(message, log->ListUser.name);
                    strcpy(buff,message);
                    userLoginCorrect = false;
                    passwordLoginCorrect = false;
                    loginCount = 0;
                    saveFile(list,"account.txt");
                } else {
                    char hash[BUFF_SIZE];
                    encode(buff, hash);
                    if (split_alpha_num(hash, alpha, num) == 0){
                        char message[] = "Eror";
                        strcpy(buff,message);
                    } else{
                        List *p = find(list, log->ListUser.name);
                        printf("%s\n",p->ListUser.name);
                        strcpy(p->ListUser.pass,buff);
                        sprintf(buff, "%s %s", alpha, num);
                        char server_reply[BUFF_SIZE] = "Reply from server: ";
                        strcat(server_reply,buff);
                        strcpy(buff, server_reply);
                    }
                }
            } else {
                if(!userLoginCorrect) {
                    log = find(list, buff);
                    if(log!=NULL) {
                        strcpy(buff,(log->ListUser.status == 0) ? "Account is blocked" : "Insert password");
                        userLoginCorrect = true;
                    } else{
                        char message[] = "Account not ready";
                        strcpy(buff, message);
                    }
                } else {
                    if(strcmp(log->ListUser.pass, buff)==0){
                        passwordLoginCorrect = true;
                        loginCount = 0;
                        char message[] = "Ok";
                        strcpy(buff,message);
                    } else {
                        loginCount++;
                        strcpy(buff, (loginCount <= 3) ? "Not ok" : "Account is blocked, enter other account");
                        if(loginCount==3) {
                            List *p = find(list, log->ListUser.name);
                            p->ListUser.status = 0;
                        } else if(loginCount > 3) {
                            saveFile(list, "account.txt");
                            loginCount = 0;
                            userLoginCorrect = false;
                            passwordLoginCorrect = false;
                        }
                    }
                }
            }
        }

        
        bytes_sent = sendto(server_sock, buff, strlen(buff) , 0, clients[0],
                            sin_size); /* send to the client welcome message */
        if (bytes_sent < 0)
            perror("\nError: ");
        else printf("Sent to client 1\n");
    }

    close(server_sock);
    return 0;
}
