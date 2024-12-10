void readFile(List **list, char* file){
	FILE* fp;
	if((fp=fopen(file,"r"))==NULL){
		printf("File %s is not exist\n",file);
		return;
	}else{
		user tempList;
		while(1){
			fscanf(fp,"%s", tempList.name);
			fscanf(fp,"%s", tempList.pass);
			fscanf(fp,"%d", &tempList.status);
			pushList(list, tempList);
			if(feof(fp)) break;
		}
	}
	fclose(fp);
}

void saveFile(List *list, char *file){
	FILE *fp = fopen(file,"w+");
	if(list==NULL) return;
	while(list->next!=NULL){
		fprintf(fp,"%s %s %d\n", list->ListUser.name, list->ListUser.pass, list
		->ListUser.status);
		list = list->next;
	}
	fclose(fp);
}