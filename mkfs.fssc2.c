/*
 * mkfs.fssc2.c
 * This file is part of FSSC tools
 *
 * Copyright (C) 2021 - adazem009
 *
 * FSSC tools is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * FSSC tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FSSC tools. If not, see <http://www.gnu.org/licenses/>.
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<dirent.h>
#include<stdbool.h>
void print_usage(char *cmdarg)
{
	printf("Usage:\n%s <file path> --root <dir path> --attributes <file path>\n",cmdarg);
}
char *_getdata(char *file)
{
	FILE *fr = fopen(file,"r");
	int ret_alloc=16, line_alloc=16;
	char *ret = (char*) malloc(ret_alloc);
	char *line = (char*) malloc(line_alloc);
	char length[1024];
	strcpy(ret,"");
	char c='\0';
	while(c != EOF)
	{
		// Read line
		strcpy(line,"");
		c='\0';
		while((c != '\n') && (c != EOF))
		{
			c=getc(fr);
			if((c != '\n') && (c != EOF))
			{
				if((strlen(line)+2) > line_alloc)
				{
					line_alloc=strlen(line)+2;
					line = (char*) realloc(line,line_alloc);
				}
				strncat(line,&c,1);
			}
		}
		if(c != EOF)
		{
			sprintf(length,"%ld",strlen(line));
			if((strlen(ret) + strlen(length) + strlen(line) + 2) > ret_alloc)
			{
				ret_alloc = strlen(ret) + strlen(length) + strlen(line) + 2;
				ret = (char*) realloc(ret,ret_alloc);
			}
			strcat(ret,length);
			strcat(ret,";");
			strcat(ret,line);
		}
	}
	fclose(fr);
	return ret;
}
char *checkdir(char *root, char *dirn, char *prefix, bool inroot, char *cmdarg, char *attlist, bool print)
{
	DIR *dr = opendir(dirn);
	DIR *sub;
	FILE *att;
	int i, count=0, path_alloc=16, newcurrent_alloc=strlen(dirn)+2, newprefix_alloc=16, tmp_alloc=16, todo_alloc=16, cur_alloc=16, files_alloc=16;
	char *path = (char*) malloc(path_alloc);
	char *newcurrent = (char*) malloc(newcurrent_alloc);
	char *newprefix = (char*) malloc(newprefix_alloc);
	char *tmp = (char*) malloc(tmp_alloc);
	char *todo = (char*) malloc(todo_alloc);
	char *cur = (char*) malloc(cur_alloc);
	char *files = (char*) malloc(files_alloc);
	strcpy(todo,"");
	char *strtoret;
	char c,count_str[16];
	bool foundatt;
	if(strcmp(prefix,"") == 0)
		strcpy(newcurrent,"/");
	else
		strcpy(newcurrent,prefix);
	strcat(newcurrent,"\n");
	strcpy(files,"");
	struct dirent *dir;
	if(dr)
	{
		while((dir=readdir(dr)) != NULL)
		{
			if((strcmp(dir->d_name,".") != 0) && (strcmp(dir->d_name,"..") != 0))
			{
				if((strlen(dirn) + strlen(dir->d_name) + 2) > path_alloc)
				{
					path_alloc = strlen(dirn) + strlen(dir->d_name) + 2;
					path = (char*) realloc(path,path_alloc);
				}
				sprintf(path,"%s/%s",dirn,dir->d_name);
				sub = opendir(path);
				// Allocate memory for file type
				if((strlen(files) + 3) > files_alloc)
				{
					files_alloc = strlen(files) + 3;
					files = (char*) realloc(files,files_alloc);
				}
				if(sub)
				{
					closedir(sub);
					// Add dir to todo list
					if((strlen(todo) + strlen(dir->d_name) + 2) > todo_alloc)
					{
						todo_alloc = strlen(todo) + strlen(dir->d_name) + 2;
						todo = (char*) realloc(todo,todo_alloc);
					}
					strcat(todo,dir->d_name);
					strcat(todo,"\n");
					// Add type (dir/file)
					strcat(files,"0\n");
				}
				else
				{
					// Add type (dir/file)
					strcat(files,"1\n");
				}
				// Add file name
				if((strlen(files) + strlen(dir->d_name) + 2) > files_alloc)
				{
					files_alloc = strlen(files) + strlen(dir->d_name) + 2;
					files = (char*) realloc(files,files_alloc);
				}
				strcat(files,dir->d_name);
				strcat(files,"\n");
				// Add file data
				if((strlen(files) + strlen(_getdata(path)) + 2) > files_alloc)
				{
					files_alloc = strlen(files) + strlen(_getdata(path)) + 2;
					files = (char*) realloc(files,files_alloc);
				}
				strcat(files,_getdata(path));
				strcat(files,"\n");
				// Add file permissions, owner, group and attributes
				if((strlen(prefix) + strlen(dir->d_name) + 2) > path_alloc)
				{
					path_alloc = strlen(prefix) + strlen(dir->d_name) + 2;
					path = (char*) realloc(path,path_alloc);
				}
				sprintf(path,"%s/%s",prefix,dir->d_name);
				att = fopen(attlist,"r");
				foundatt=false;
				c='\0';
				while(c != EOF)
				{
					// Read path
					if(c != EOF)
						c='\0';
					strcpy(tmp,"");
					while((c != '\n') && (c != EOF))
					{
						c=getc(att);
						if((c != '\n') && (c != EOF))
						{
							if((strlen(tmp)+2) > tmp_alloc)
							{
								tmp_alloc=strlen(tmp)+2;
								tmp = (char*) realloc(tmp,tmp_alloc);
							}
							strncat(tmp,&c,1);
						}
					}
					if(strcmp(tmp,path) == 0)
					{
						for(i=0;i<4;i++)
						{
							// Read permissions/owner/group
							if(c != EOF)
								c='\0';
							strcpy(tmp,"");
							while((c != '\n') && (c != EOF))
							{
								c=getc(att);
								if((c != '\n') && (c != EOF))
								{
									if((strlen(tmp)+2) > tmp_alloc)
									{
										tmp_alloc=strlen(tmp)+2;
										tmp = (char*) realloc(tmp,tmp_alloc);
									}
									strncat(tmp,&c,1);
								}
							}
							// Add permissions/owner/group
							if((strlen(files) + strlen(tmp) + 2) > files_alloc)
							{
								files_alloc = strlen(files) + strlen(tmp) + 2;
								files = (char*) realloc(files,files_alloc);
							}
							strcat(files,tmp);
							strcat(files,"\n");
						}
						// Stop this loop
						foundatt=true;
						c=EOF;
					}
					else
					{
						// Skip permissions, owner and group
						for(i=0;i<4;i++)
						{
							if(c != EOF)
								c='\0';
							while((c != '\n') && (c != EOF))
								c=getc(att);
						}
					}
				}
				if(!(foundatt))
				{
					if(print)
						printf("warning: SECURITY ISSUE: Permissions for '%s' aren't defined. Using 'rwxrwxrwx;root;root;'\n",path);
					// Add default permissions
					if((strlen(files) + 23) > files_alloc)
					{
						files_alloc = strlen(files) + 23;
						files = (char*) realloc(files,files_alloc);
					}
					strcat(files,"rwxrwxrwx\nroot\nroot\n\n");
				}
				fclose(att);
				count++;
			}
		}
	}
	else
	{
		if(inroot)
		{
			printf("%s: %s: %s\n",cmdarg,dirn,strerror(errno));
			exit(2);
		}
		strtoret = (char*) malloc(sizeof(newcurrent));
		strcpy(strtoret,newcurrent);
		return strtoret;
	}
	// Add file count
	sprintf(count_str,"%d",count);
	if((strlen(newcurrent) + strlen(count_str) + strlen(files) + 2) > newcurrent_alloc)
	{
		newcurrent_alloc = strlen(newcurrent) + strlen(count_str) + strlen(files) + 2;
		newcurrent = (char*) realloc(newcurrent,newcurrent_alloc);
	}
	strcat(newcurrent,count_str);
	strcat(newcurrent,"\n");
	strcat(newcurrent,files);
	// Read todo list
	for(i=0; i < strlen(todo); i++)
	{
		// Read line
		strcpy(cur,"");
		c='\0';
		while(c != '\n')
		{
			c=todo[i];
			if(c != '\n')
			{
				if((strlen(cur)+2) > cur_alloc)
				{
					cur_alloc=strlen(cur)+2;
					cur = (char*) realloc(cur,cur_alloc);
				}
				strncat(cur,&todo[i],1);
				i++;
			}
		}
		if((strlen(prefix) + strlen(cur) + 5) > newprefix_alloc)
		{
			newprefix_alloc = strlen(prefix) + strlen(cur) + 5;
			newprefix = (char*) realloc(newprefix,newprefix_alloc);
		}
		sprintf(newprefix,"%s/%s",prefix,cur);
		if((strlen(dirn) + strlen(cur) + 2) > path_alloc)
		{
			newprefix_alloc = strlen(dirn) + strlen(cur) + 2;
			path = (char*) realloc(path,newprefix_alloc);
		}
		sprintf(path,"%s/%s",dirn,cur);
		if((strlen(checkdir(root,path,newprefix,false,cmdarg,attlist,false)) + 1) > tmp_alloc)
		{
			tmp_alloc = strlen(checkdir(root,path,newprefix,false,cmdarg,attlist,false)) + 1;
			tmp = (char*) realloc(tmp,tmp_alloc);
		}
		strcpy(tmp,checkdir(root,path,newprefix,false,cmdarg,attlist,true));
		if((strlen(newcurrent) + strlen(tmp) + 1) > newcurrent_alloc)
		{
			newcurrent_alloc = strlen(newcurrent) + strlen(tmp) + 1;
			newcurrent = (char*) realloc(newcurrent,newcurrent_alloc);
		}
		strcat(newcurrent,tmp);
	}
	closedir(dr);
	strtoret = (char*) malloc(strlen(newcurrent)+1);
	strcpy(strtoret,newcurrent);
	return strtoret;
}
int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		print_usage(argv[0]);
		exit(1);
	}
	int i,current_path_alloc=16;
	char filename[255],rootdir[255],attfile[255];
	strcpy(filename,"");
	strcpy(rootdir,"");
	strcpy(attfile,"attributes.list");
	char *current_path = (char*) malloc(current_path_alloc);
	char *tree;
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] == '-')
		{
			if(strcmp(argv[i],"--root") == 0)
			{
				if(i+1 == argc)
				{
					printf("%s: Missing root directory path\n",argv[0]);
					exit(1);
				}
				i++;
				strcpy(rootdir,argv[i]);
			}
			else if(strcmp(argv[i],"--attributes") == 0)
			{
				if(i+1 == argc)
				{
					printf("%s: Missing file operand\n",argv[0]);
					exit(1);
				}
				i++;
				strcpy(attfile,argv[i]);
			}
			else
			{
				printf("%s: Unknown option '%s'\n",argv[0],argv[i]);
				print_usage(argv[0]);
				exit(1);
			}
		}
		else
		{
			if(strcmp(filename,"") == 0)
				strcpy(filename,argv[i]);
			else
			{
				print_usage(argv[0]);
				exit(1);
			}
		}
	}
	if(strcmp(filename,"") == 0)
	{
		printf("Missing file name\n");
		print_usage(argv[0]);
		exit(1);
	}
	if(strcmp(rootdir,"") == 0)
	{
		printf("%s: Root directory path not specified\n",argv[0]);
		print_usage(argv[0]);
		exit(3);
	}
	FILE *ar = fopen(attfile,"r");
	if(errno != 0)
	{
		printf("%s: %s: %s\n",argv[0],attfile,strerror(errno));
		exit(2);
	}
	fclose(ar);
	FILE *fw = fopen(filename,"w");
	if(errno != 0)
	{
		printf("%s: %s: %s\n",argv[0],filename,strerror(errno));
		exit(2);
	}
	// Add FSSC header
	fprintf(fw,"FSSC2\n");
	// Add filesystem tree
	tree = (char*) malloc(strlen(checkdir(rootdir,rootdir,"",true,argv[0],attfile,false))+1);
	fprintf(fw,"%s",checkdir(rootdir,rootdir,"",true,argv[0],attfile,true));
	fclose(fw);
	return 0;
}
