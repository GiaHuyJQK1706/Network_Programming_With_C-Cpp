#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct account {
    int status;           
    char userName[60];    
    char password[40];    
} account;

typedef struct node {
    account acc;          
    int statusLogin;      
    struct node *next;    
} node;

void pushList(node **root, account acc);
void deleteElement(node **root, char *userName);
void showList(node *root);
void freeList(node *head);