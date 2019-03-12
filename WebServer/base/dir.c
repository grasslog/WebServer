#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

void showallfile(char* filepath);

int main(int argc, char* argc[])
{
	if(argc != 2){
		printf("Error. Please input filepath.\n");
		exit(-1);
	}
	showallfile(argv[1]);
}

void showallfile(char* filepath)
{
	struct stat st;
	DIR* dp;
	struct dirent* dirp;
	char* pstr;
	if(lstat(filepath, &st) == -1){
		perror("lstat() error");
		exit(-1);
	}
	if(S_ISDIR(st.st_node) == 0){
		printf("File: %s\n", filepath);
	}else{
		printf("Directory: %s\n", filepath);
		pstr = filepath + strlen(filepath);
		*pstr++ = '/';
		*pstr = 0;
		if((dp = opendir(filepath)) == NULL){
			printf("opendir() error");
			exit(-1);
		}
		while((dirp=readdir(dp))!=NULL)
		{
			if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0){
				continue;
			}
			strcpy(pstr, dirp->d_name);
			showallfile(filepath);
		}
	}
}