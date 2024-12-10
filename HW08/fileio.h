#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

void readFile(List **list, char* file) {
    FILE* fp = fopen(file, "r");
    if (fp == NULL) {
        printf("File %s is not found\n", file);
        return;
    }
    user tempList;
    while (fscanf(fp, "%s %s %d", tempList.name, tempList.pass, &tempList.status) != EOF) {
        pushList(list, tempList);
    }
    fclose(fp);
}

void saveFile(List *list, char *file) {
    FILE *fp = fopen(file, "w+");
    while (list != NULL) {
        fprintf(fp, "%s %s %d\n", list->ListUser.name, list->ListUser.pass, list->ListUser.status);
        list = list->next;
    }
    fclose(fp);
}

#endif
