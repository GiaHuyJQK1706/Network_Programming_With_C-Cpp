#include "list.h"

void pushList(node **root, account acc) {
    node* newNode = (node*)malloc(sizeof(node));
    newNode->acc = acc;
    newNode->next = *root;
    *root = newNode;
}

void deleteElement(node **root, char *userName) {
    node *tmp, *tmp2;
    if (strcmp((*root)->acc.userName, userName) == 0) {
        tmp = *root;
        *root = (*root)->next;
        free(tmp);
    } else {
        tmp = *root;
        tmp2 = tmp->next;
        while (tmp2 != NULL) {
            if (strcmp(tmp2->acc.userName, userName) == 0) {
                tmp->next = tmp2->next;
                free(tmp2);
                break;
            }
            tmp = tmp->next;
            tmp2 = tmp2->next;
        }
    }
}

void showList(node *root) {
    while (root != NULL) {
        printf("%-60s:%-40s:%d\n", root->acc.userName, root->acc.password, root->acc.status);
        root = root->next;
    }
}

void freeList(node *head) {
    node* tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}
