/*
 * mkfpt.c
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
#include<stdbool.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
void print_usage(char *cmdarg)
{
	printf("Usage:\n%s <output file path> --config <path to config file>\n",cmdarg);
}
char *readline(FILE *fr, int line)
{
	int i=0, out_alloc=16;
	char *out = (char*) malloc(out_alloc);
	char c='\0';
	rewind(fr);
	while(c != EOF)
	{
		// Read line
		if(i == line)
			strcpy(out,"");
		if(c != EOF)
			c='\0';
		while((c != '\n') && (c != EOF))
		{
			c=getc(fr);
			if(i == line)
			{
				if((c != '\n') && (c != EOF))
				{
					if((strlen(out)+2) > out_alloc)
					{
						out_alloc=strlen(out)+2;
						out = (char*) realloc(out,out_alloc);
					}
					strncat(out,&c,1);
				}
			}
		}
		i++;
	}
	return out;
}
char *getvar(char *input)
{
	char *out = (char*) malloc(strlen(input)+1);
	strcpy(out,"");
	int i;
	for(i=0; i < strlen(input); i++)
	{
		if(input[i] == '=')
			break;
		else
			strncat(out,&input[i],1);
	}
	return out;
}
char *getpart(char *input, int partn)
{
	char *out = (char*) malloc(strlen(input)+1);
	char *part = (char*) malloc(strlen(input)+1);
	strcpy(out,"");
	strcpy(part,"");
	int i;
	for(i=0; i < strlen(input); i++)
	{
		if(input[i] == '.')
		{
			if(partn == 1)
			{
				strcpy(out,part);
				break;
			}
			strcpy(part,"");
		}
		else
			strncat(part,&input[i],1);
	}
	if(partn == 2)
		strcpy(out,part);
	return out;
}
char *getvalue(char *input)
{
	char *out = (char*) malloc(strlen(input)+1);
	strcpy(out,"");
	bool wasvar=false;
	int i;
	for(i=0; i < strlen(input); i++)
	{
		if(input[i] == '=')
			wasvar=true;
		else
			if(wasvar)
				strncat(out,&input[i],1);
	}
	return out;
}
int linecount(FILE *fr)
{
	int i=0;
	char c;
	rewind(fr);
	while((c=getc(fr)) != EOF)
		if(c == '\n')
			i++;
	return i;
}
int main(int argc, char *argv[])
{
	int i;
	char *filename,c;
	char *configfile = (char*) malloc(10);
	strcpy(configfile,"disk.conf");
	bool outfile=false;
	bool nombr=false;
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] == '-')
		{
			if(strcmp(argv[i],"--config") == 0)
			{
				if(i+1 == argc)
				{
					printf("%s: missing file operand\n",argv[0]);
					exit(1);
				}
				i++;
				configfile = (char*) realloc(configfile,strlen(argv[i])+1);
				strcpy(configfile,argv[i]);
			}
			else if(strcmp(argv[i],"--no-mbr") == 0)
			{
				nombr=true;
			}
			else
			{
				printf("%s: unknown option '%s'\n",argv[0],argv[i]);
				print_usage(argv[0]);
				exit(1);
			}
		}
		else
		{
			if(outfile)
			{
				print_usage(argv[0]);
				exit(1);
			}
			else
			{
				outfile=true;
				filename = (char*) malloc(strlen(argv[i])+1);
				strcpy(filename,argv[i]);
			}
		}
	}
	if(!(outfile))
	{
		print_usage(argv[0]);
		exit(1);
	}
	FILE *ow, *cr, *tmp;
	// Open output file
	ow = fopen(filename,"w");
	if(errno != 0)
	{
		printf("%s: %s: %s\n",argv[0],filename,strerror(errno));
		exit(2);
	}
	// Open config file
	cr = fopen(configfile,"r");
	if(errno != 0)
	{
		printf("%s: %s: %s\n",argv[0],configfile,strerror(errno));
		exit(2);
	}
	if(!(nombr))
	{
		// Add MBR
		if(strcmp(getvar(readline(cr,0)),"mbr.image") != 0)
		{
			printf("%s: expected mbr.image, but got '%s'\n",argv[0],getvar(readline(cr,0)));
			exit(10);
		}
		tmp = fopen(getvalue(readline(cr,0)),"r");
		if(errno != 0)
		{
			printf("%s: '%s': %s\n",argv[0],getvalue(readline(cr,0)),strerror(errno));
			exit(2);
		}
		i=0;
		while((c=getc(tmp)) != EOF)
		{
			putc(c,ow);
			i++;
		}
		if(i != 4096)
		{
			printf("%s: invalid MBR size '%d', it must be 4096\n",argv[0],i);
		}
		fclose(tmp);
		i=1;
	}
	else
		i=0;
	// Add partitions
	int partn=0, linestr_alloc=16, filestr_alloc=16;
	int linec=linecount(cr);
	char strnum[16];
	char *linestr = (char*) malloc(linestr_alloc);
	char *filestr = (char*) malloc(filestr_alloc);
	for(i=i; i < linec; i++)
	{
		sprintf(strnum,"%d",partn);
		if(strcmp(getpart(getvar(readline(cr,i)),1),strnum) == 0)
		{
			// Add name
			if(strcmp(getpart(getvar(readline(cr,i)),2),"name") == 0)
			{
				fprintf(ow,"%ld;%s",strlen(getvalue(readline(cr,i))),getvalue(readline(cr,i)));
			}
			else
			{
				printf("%s: expected '%d.name', but got '%s'\n",argv[0],partn,getvar(readline(cr,i)));
				exit(12);
			}
			i++;
			// Add size
			if(strcmp(getpart(getvar(readline(cr,i)),2),"size") == 0)
			{
				fprintf(ow,"%ld;%s",strlen(getvalue(readline(cr,i))),getvalue(readline(cr,i)));
			}
			else
			{
				printf("%s: expected '%d.size', but got '%s'\n",argv[0],partn,getvar(readline(cr,i)));
				exit(12);
			}
			i++;
			// Add content
			if(strcmp(getpart(getvar(readline(cr,i)),2),"image") == 0)
			{
				tmp = fopen(getvalue(readline(cr,i)),"r");
				strcpy(filestr,"");
				c='\0';
				while(c != EOF)
				{
					// Read line
					strcpy(linestr,"");
					if(c != EOF)
						c='\0';
					while((c != '\n') && (c != EOF))
					{
						c=getc(tmp);
						if((c != '\n') && (c != EOF))
						{
							if((strlen(linestr)+2) > linestr_alloc)
							{
								linestr_alloc=strlen(linestr)+2;
								linestr = (char*) realloc(linestr,linestr_alloc);
							}
							strncat(linestr,&c,1);
						}
					}
					if(c != EOF)
					{
						// Add strlen(linestr);linestr
						sprintf(strnum,"%ld",strlen(linestr));
						if((strlen(filestr) + strlen(strnum) + strlen(linestr) + 4) > linestr_alloc)
						{
							filestr_alloc = strlen(filestr) + strlen(strnum) + strlen(linestr) + 4;
							filestr = (char*) realloc(filestr,filestr_alloc);
						}
						strcat(filestr,strnum);
						strcat(filestr,";");
						strcat(filestr,linestr);
					}
				}
				fprintf(ow,"%ld;%s",strlen(filestr),filestr);
				fclose(tmp);
			}
			else
			{
				printf("%s: expected '%d.image', but got '%s'\n",argv[0],partn,getvar(readline(cr,i)));
				exit(12);
			}
			partn++;
		}
		else
		{
			printf("%s: expected partition %d, but got %s\n",argv[0],partn,getpart(getvar(readline(cr,i)),1));
			exit(11);
		}
	}
	fclose(ow);
	fclose(cr);
	return 0;
}
