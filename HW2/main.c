#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <ctype.h>

int hostnameToIp(char *param) {
    struct hostent *he;
    struct in_addr **addr_list;
    if ((he = gethostbyname(param)) == NULL) {
        printf("Not found information\n");
        return 1;
    }
    printf("Official IP: %s\n", inet_ntoa(*(struct in_addr *)he->h_addr));
    addr_list = (struct in_addr **)he->h_addr_list;
    printf("Alias IP: \n");
    for (int i = 1; addr_list[i] != NULL; i++) {
        printf("%s\n", inet_ntoa(*addr_list[i]));
    }
    return 0;
}

int ipToHostname(char *param, struct in_addr ip) {
    struct hostent *hp;
    if ((hp = gethostbyaddr((const void *)&ip, sizeof ip, AF_INET)) == NULL) {
        printf("Not found information\n");
        return 1;
    }
    printf("Official name: %s\n", hp->h_name);
    printf("Alias name:\n");
    for (int i = 0; hp->h_aliases[i] != NULL; i++) {
        printf("%s\n", hp->h_aliases[i]);
    }
    return 0;
}

void checkDomainDochai(char *domain) {
    char api_key[] = "YOUR_API_KEY";  // Dien khoa API cua ban
    //char api_key[] = "another_api_key";
    char command_api[512];
    char result_file[] = "result.txt"; // File tam de luu ket qua API

    snprintf(command_api, sizeof(command_api), 
             "curl --silent \"https://www.virustotal.com/vtapi/v2/domain/report?apikey=%s&domain=%s\" > %s", 
             api_key, domain, result_file);
    system(command_api);
    FILE *fp = fopen(result_file, "r");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return;
    }
    char buffer[65536];
    int dochai = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "\"positives\":") != NULL) {
            int positives;
            sscanf(buffer, " \"positives\": %d,", &positives);
            if (positives > 0) {
                dochai = 1;
            }
            break;
        }
    }
    fclose(fp);
    remove(result_file);
    if (dochai) {
        printf("Ten mien nay doc hai\n");
    } else {
        printf("Ten mien nay khong doc hai\n");
    }
}

int main(int argc, char *argv[]) {
    struct in_addr ip;
    if (argc != 2) {
        printf("error: wrong form\n");
        return 1;
    }
    if (!inet_pton(AF_INET, argv[1], &ip)) {
        int result = hostnameToIp(argv[1]);
        if (result == 0) {
            checkDomainDochai(argv[1]);
        }
    } else {
        int result = ipToHostname(argv[1], ip);
        if (result == 0) {
            checkDomainDochai(argv[1]);
        }
    }
    return 0;
}
