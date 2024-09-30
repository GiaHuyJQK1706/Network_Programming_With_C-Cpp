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
#include <curl/curl.h>

struct memory {
    char *response;
    size_t size;
};

static size_t callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;
    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if (ptr == NULL) {
        free(mem->response);
        printf("Not enough memory\n");
        return 0;
    }
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
    return realsize;
}

void crawlData(const char *url) {
    CURL *curl;
    CURLcode res;
    struct memory chunk;
    chunk.response = malloc(1); 
    chunk.size = 0;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl) {
        char full_url[2048];
        snprintf(full_url, sizeof(full_url), "%s%s", "http://", url);

        curl_easy_setopt(curl, CURLOPT_URL, full_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3");

        if (curl_easy_perform(curl) != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() error: %s\n", curl_easy_strerror(res));
        } else {
            printf("Response:\n%s\n", chunk.response);
            FILE *link_file = fopen("links.csv", "w+");
            FILE *text_file = fopen("text.csv", "w+");
            FILE *video_file = fopen("videos.csv", "w+");
            char *data = chunk.response;
            char *ptr = data;
            //Link
            while ((ptr = strstr(ptr, "<a ")) != NULL) {
                char *start = strstr(ptr, "href=\"");
                if (start) {
                    start += 6;
                    char *end = strchr(start, '\"');
                    if (end) {
                        char link[65536];
                        strncpy(link, start, end - start);
                        link[end - start] = '\0';
                        fprintf(link_file, "%s\n", link);
                    }
                }
                ptr++;
            }
            //Text
            ptr = data;
            while ((ptr = strstr(ptr, "<p>")) != NULL) {
                char *start = ptr + 3;
                char *end = strstr(start, "</p>");
                if (end) {
                    char text[65536];
                    strncpy(text, start, end - start);
                    text[end - start] = '\0';
                    fprintf(text_file, "%s\n", text);
                }
                ptr++;
            }
            ptr = data;
            while ((ptr = strstr(ptr, "<h1>")) != NULL) {
                char *start = ptr + 4;
                char *end = strstr(start, "</h1>");
                if (end) {
                    char text[65536];
                    strncpy(text, start, end - start);
                    text[end - start] = '\0';
                    fprintf(text_file, "%s\n", text);
                }
                ptr++;
            }
            ptr = data;
            while ((ptr = strstr(ptr, "<h2>")) != NULL) {
                char *start = ptr + 4;
                char *end = strstr(start, "</h2>");
                if (end) {
                    char text[65536];
                    strncpy(text, start, end - start);
                    text[end - start] = '\0';
                    fprintf(text_file, "%s\n", text);
                }
                ptr++;
            }
            ptr = data;
            while ((ptr = strstr(ptr, "<h3>")) != NULL) {
                char *start = ptr + 4;
                char *end = strstr(start, "</h3>");
                if (end) {
                    char text[65536];
                    strncpy(text, start, end - start);
                    text[end - start] = '\0';
                    fprintf(text_file, "%s\n", text);
                }
                ptr++;
            }
            //Video
            ptr = data;
            while ((ptr = strstr(ptr, "<iframe ")) != NULL || (ptr = strstr(ptr, "<video ")) != NULL) {
                char *start = strstr(ptr, "src=\"");
                if (start) {
                    start += 5;
                    char *end = strchr(start, '\"');
                    if (end) {
                        char video[65536];
                        strncpy(video, start, end - start);
                        video[end - start] = '\0';
                        fprintf(video_file, "%s\n", video);
                    }
                }
                ptr++;
            }
            fclose(link_file);
            fclose(text_file);
            fclose(video_file);
        }
        curl_easy_cleanup(curl);
    }
    free(chunk.response);
    curl_global_cleanup();
}

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
    char api_key[] = "9ac447580cc88e56791b8be0b35c67c8605815b565e79af0b419914914f30c57";  
    char command_api[512];
    char result_file[] = "result.txt";
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
            crawlData(argv[1]);
        }
    } else {
        int result = ipToHostname(argv[1], ip);
        if (result == 0) {
            checkDomainDochai(argv[1]);
            crawlData(argv[1]);
        }
    }
    return 0;
}
