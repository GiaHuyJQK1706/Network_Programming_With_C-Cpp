typedef struct {
	char name[20];
	char pass[20];
	int status;
} user;

typedef struct listNode {
	user ListUser;
	struct listNode *next;
	struct listNode *prev;
} List;

List* createList(user listUser);
void pushList(List **list, user listUser);
List* find(List *list, char* nameUser);
void removeList(List **list, char* nameUser);

List *createList (user listUser){
	List* newList = (List*)malloc(sizeof(List));
	strcpy(newList->ListUser.name, listUser.name);
	strcpy(newList->ListUser.pass, listUser.pass);
	newList->ListUser.status = listUser.status;
	newList->next = NULL;
	newList->prev = NULL;
	return newList;
}

void pushList (List **list, user listUser){
	List* newList = createList(listUser);
	if((*list)==NULL) *list = newList;
	else{
		List *p = *list;
		while(p->next!=NULL) p = p->next;
		p->next = newList;
		newList->prev = p;
	}
}

List* find(List *list, char* nameUser){
	while(list!=NULL){
		if(strcmp(list->ListUser.name, nameUser)==0){
			return list;
		}
		list = list->next;
	}
	return NULL;
} 

void removeList(List **list,char *nameUser) {
	if ((*list)==NULL) return;
	List *p;
	if (strcmp((*list)->ListUser.name, nameUser)==0) {
		p = *list;
		*list = (*list)->next;
		if((*list)!=NULL) (*list)->prev = NULL;
		free(p);
	} else {
		p = *list;
		while(p->next!=NULL){
			printf("%s", p->ListUser.name);
			if(strcmp(p->ListUser.name, nameUser)==0) break;
			p = p->next;
		}
		if(p==NULL) return;
		p->prev->next = p->next;
		free(p);
	}
}
