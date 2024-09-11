#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "list.h"

FILE *f;

void readFile(node **root) {
    account tmp;
    f = fopen("user.txt", "r+");
    if (f != NULL) {
        while (fscanf(f, "%[^:]:%[^:]:%d\n", tmp.userName, tmp.password, &tmp.status) != EOF) {
            pushList(root, tmp);
        }
    } else {
        printf("File not found!\n");
    }
    fclose(f);
}

void writeFile(node *root) {
    f = fopen("user.txt", "wt");
    if (f != NULL) {
        while (root != NULL) {
            fprintf(f, "%s:%s:%d\n", root->acc.userName, root->acc.password, root->acc.status);
            root = root->next;
        }
        fclose(f);
    } else {
        printf("Error write file!\n");
    }
}

void registerAcc(node **root) {
    char name[60], pass[40];
    node* r = *root;
    int check = 0;
    printf("----------Register----------\n");
    printf("Username: ");
    scanf("%s", name);
    while (r != NULL) {
        if (strcmp(r->acc.userName, name) == 0) {
            check = 1;
            break;
        }
        r = r->next;
    }
    if (check == 1) {
        printf("Account already exists!\n");
    } else {
        printf("Password: ");
        scanf("%s", pass);
        account a;
        strcpy(a.userName, name);
        strcpy(a.password, pass);
        a.status = 1;
        pushList(root, a);
        writeFile(*root);
        printf("Successful registration!\n");
    }
}

void login(node *root) {
    char name[60], pass[40];
    int check = 0, attempt = 0;
    node *r = root;
    printf("-------------Login------------\n");
    printf("Username: ");
    scanf("%s", name);
    while (r != NULL) {
        if (strcmp(r->acc.userName, name) == 0) {
            check = 1;
            break;
        }
        r = r->next;
    }
    if (check == 0) {
        printf("Cannot find account!!!\n");
    } else if (r->acc.status == 0) {
        printf("Account is blocked!\n");
    } else {
        do {
            printf("Password: ");
            scanf("%s", pass);
            if (strcmp(r->acc.password, pass) != 0) {
                attempt++;
                if (attempt == 3) {
                    r->acc.status = 0;
                    writeFile(root);
                    printf("Password is incorrect. Account is blocked!\n");
                    break;
                }
                printf("Incorrect password! Try again.\n");
            } else {
                printf("Hello %s\n", name);
                r->statusLogin = 1;
                break;
            }
        } while (attempt < 3);
    }
}

void search(node *root) {
    char name[60];
    int flag = 0;
    node *r = root;
    printf("-------------Search------------\n");
    printf("Username: ");
    scanf("%s", name);
    while (r != NULL) {
        if (strcmp(r->acc.userName, name) == 0) {
            flag = 1;
            break;
        }
        r = r->next;
    }
    if (flag != 1) {
        printf("Cannot find account!!!\n");
    } else if (r->acc.status == 0) {
        printf("Account is blocked!\n");
    } else {
        printf("Account is active!\n");
    }
}

void logout(node *root) {
    char name[60];
    int flag = 0;
    node *r = root;
    printf("-------------Logout------------\n");
    printf("Username: ");
    scanf("%s", name);
    while (r != NULL) {
        if (strcmp(r->acc.userName, name) == 0) {
            flag = 1;
            break;
        }
        r = r->next;
    }
    if (flag != 1) {
        printf("Cannot find account!!!\n");
    } else if (r->statusLogin == 0) {
        printf("Account is not logged in\n");
    } else {
        printf("Goodbye %s!\n", r->acc.userName);
        r->statusLogin = 0;
    }
}

void printMenu(){
    printf("           USER MANAGEMENT PROGRAM           \n");
    printf("---------------------------------------------\n");
    printf("1. Register\n");
    printf("2. Sign in\n");
    printf("3. Search\n");
    printf("4. Sign out\n");
    printf("Your choice (1-4, other to quit): ");
}

int main(){
    node *root = NULL;
    int luachon;
    readFile(&root);
    do {
        printMenu();
        scanf("%d", &luachon);

        if (luachon == 1) {
            registerAcc(&root);
        } else if (luachon == 2) {
            login(root);
        } else if (luachon == 3) {
            search(root);
        } else if (luachon == 4) {
            logout(root);
        }
    } while (luachon >= 1 && luachon <= 4);
    freeList(root);
    return 0;
}


