#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include "myHeader.h"
// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_ARG 128

char *tokens[100];
char lastLocation[100];
int main (int argc, char ** argv, char **envp) {

  int finished = 0;
  char *prompt = "  320sh> ";
  char cmd[MAX_INPUT];;
  int index = 0;

 splitPath(tokens);
 

  while (!finished) {
    char *cursor;
    char *originalPos;
    char last_char;
    int rv;
    int count;


    // Print the prompt
    char *pwd = malloc(100);
	getcwd(pwd,100);
	write(1,"[",1);
	write(1,pwd,strlen(pwd));
	write(1,"]",1);
	free(pwd);
    
    rv = write(1, prompt, strlen(prompt));;
    if (!rv) { 
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, 
	  cursor = cmd, last_char = 1;
	  rv 
	  && (++count < (MAX_INPUT-1))
	  && (last_char != '\n');
	cursor++) { 
      originalPos=cursor;
      rv = read(0, cursor, 1);
  	  
      last_char = *cursor;
      
      handleCursor(cmd,originalPos,last_char);


      if(last_char == 3) {
        write(1, "^c", 2);
     
      } else {
      	write(1, &last_char, 1);
		cmd[index]=last_char;
		index ++;
      }

    } 

    *cursor = '\0';


    if (!rv) { 
      finished = 1;
      break;
    }

	
    // Execute the command, handling built-in commands separately 
    // Just echo the command line for now
    // write(1, cmd, strnlen(cmd, MAX_INPUT));
	eva(cmd);
  }

  return 0;
}

void eva(char* cmd){

		char *argv[MAX_ARG]; /*Argument list*/
		char buf[MAX_INPUT]; /*Copy of command line*/
		int job;			 /*hold job type, background if 0, foreground otherwise*/
		pid_t pid;			 /*new process id*/
	
		
		strcpy(buf, cmd);

		/*parse command line*/
		job = parse(buf,argv);
		 write(1,"s",1);
		int i = 0;
		while(argv[i]!=NULL){
			printf("argv%d is :%s  ",i,argv[i]);
			i++;
		}

		printf("job = %d\n",job);
		  
		/*check if command if empty*/
		if(argv[0] == NULL){
			return;
		}
		
		int build_in = 0;
		/*check to see if command is buildin */
		buildIn(argv,&build_in);

			if(build_in == 0){		
			
			/*find path of binary file*/
				char newPath[1028] = "";
				
				findPath(argv[0],newPath);
				//printf("path is : %s\n",newPath);
				if(newPath != NULL){
					/*create a child and invoke function if path is not null*/
					if((pid = fork()) == 0){
						/*child process*/
					//	char *env[] = {NULL};
						#ifdef d
							fprintf(stderr,"RUNNING : %s",cmd);
						#endif

						if(execve(newPath,argv,NULL) < 0)
							printf("%s: command not found\n", argv[0]);
						
						exit(0);
					}else{
						/*in parent, wait for child to finish*/
						int status = 0;
						
						if(waitpid(-1,&status,0) >= 0){
						/*child successfully reap*/
							if(WIFEXITED(status)){
							/*child terminate correctly*/
								#ifdef d
									fprintf(stderr,"ENDED : %s (ret:%d)",cmd,WEXITSTATUS(status));
								#endif
									return;
							}else{
							/*something wrong when child terminate*/
								fprintf(stderr,"Child terminated abnormally with error:%s\n",strerror(errno));
								return;
							}
						}else{
							fprintf(stderr,"ERROR ON WAITPID with error:%s\n",strerror(errno));
						}
					}
				}else{
					/*path not found*/
					fprintf(stderr,"%s : command not found",argv[0]);
					return;
				}
			}

	
			
		
}
	

int parse(char buf[],char *argv[]){


  
   const char s[1] = " ";
   char *token;
   char *str;
   int index =0;
   /* get the first token */
  
   token = strtok(buf, s);
     
   /* walk through other tokens */
   while( token != NULL ) 
   {    
     argv[index]=token;
     token = strtok(NULL, s);		   
     index++;
   }

	//set last index = NUll	
	argv[index] = NULL;
	
	//get ride of /n 
	str = argv[index-1];
	str = strtok(str, "\n");

	argv[index-1] = str;
  
	//background process condition
	if(argv[index-1] != NULL){
		if(*argv[--index]=='&'){
		argv[index] = NULL;
		return 0;
		}
	}
	//foreground process
	return 1;

	

}

void findPath(char *path,char newPath[]){


 	char fullPath[1028]="";

	int foundPath = 0;
	
	/*if path is absolute or relative, just check if file exist*/
  if(path[0]=='/' || path[0] == '.'){

	if(file_exist(path)){
		strcpy(newPath,path);
	}	
	else{ 
		newPath = NULL;
		}
	}	
	
  /*else find the absolute path*/
  else{

		int i=0;
		while(tokens[i]!=NULL){
		
		
		strcpy(fullPath,tokens[i]);
		strcat(fullPath,"/");
		strcat(fullPath,path);

			if(file_exist(fullPath)){	
				strcpy(newPath,fullPath);
				foundPath = 1;
				break;
			}
			i++;
		}
		
		if(foundPath == 0)
			newPath = NULL;
	}

	
}

int file_exist (const char *filePath)
{
  struct stat temp;   
  return (stat(filePath, &temp) == 0);
}


void buildIn(char* cmd[], int *build_In){

	if(!strcmp(cmd[0],"exit")){		
		/*exit program*/
		write(1,"PROGRAM EXITING\n",17);
		*build_In = 1;
		exit(EXIT_SUCCESS);
	}else if(!strcmp(cmd[0],"cd")){		
		/*call cd program*/		
		*build_In = 1;
		CD(cmd);
	}else if(!strcmp(cmd[0],"set")){		
		/*call set program*/
		*build_In = 1;
		SET(cmd);
	}else if(!strcmp(cmd[0],"pwd")){		
		/*call pwd program*/
		*build_In = 1;
		PWD(cmd);
	}else if(!strcmp(cmd[0],"echo")){		
		/*call echo program*/
		*build_In = 1;
		ECHO(cmd);
	}else if(!strcmp(cmd[0],"help")){		
		/*call help program*/
		*build_In = 1;
		HELP(cmd);
	}
}


void CD(char *cmd[0]){
	char *pwd = malloc(100);
	char temp[100];
	char *ret;

	getcwd(pwd,100);
 
	// if cd .. go to previous directory
	if(cmd[1]==NULL){

	strcpy(lastLocation,pwd);
	strcpy( temp,getenv("HOME"));
		if(chdir(temp)<0){
			printf("error: %s\n", strerror(errno));
		}



	}else if(strcmp(cmd[1],"..")==0){
		strcpy(lastLocation,pwd);	

		strcpy(temp,pwd);

		ret= strrchr(temp, '/');

		*ret = '\0'; 
		if(chdir(temp)<0){
			printf("error: %s\n", strerror(errno));
		}

	//if cd - go to last location
	}else if(strcmp(cmd[1],"-")==0){
	
		if(chdir(lastLocation)<0){
			printf("error: %s\n", strerror(errno));
		}
		strcpy(lastLocation,pwd);	
	

	}else if(strcmp(cmd[1],".")==0){
	


	}else{

		strcpy(lastLocation,pwd);	

		strcat(pwd,"/");
		strcat(pwd,cmd[1]);
		if(chdir(pwd)<0){
			printf("error: %s\n", strerror(errno));
		}



	}
	free(pwd);
}

void ECHO(char *cmd[0]){}


void SET(char *cmd[0]){

	int valid = 0;
	/*check if set arugments are valid*/

	if(cmd[1] == NULL){
		fprintf(stderr,"Function set should be in format name = value\n");
		return;
		}
	valid++;
	if(cmd[2] != NULL){
	if(strcmp(cmd[2],"=")){
		fprintf(stderr,"Function set should be in format name = value\n");
		return;
		}
	}
	valid++;
	if(cmd[3] == NULL){
		fprintf(stderr,"Function set should be in format name = value\n");
		return;}
	valid++;
		
	if(valid==3){
		fprintf(stderr,"name = %s, value = %s\n",cmd[1],cmd[3]);
		/*first check if value overlap, if yes concat new value with previous value ans set again*/
		char *prev = getenv(cmd[1]);
		if(prev != NULL){
			/*value already existd*/
			char newEnv[1024];
			strcpy(newEnv,cmd[3]);
			strcat(newEnv,prev);
			if(setenv(cmd[1],newEnv,1))
				fprintf(stderr,"Error in set env with error:%s\n",strerror(errno));
			//fprintf(stderr,"new Path: %s\n",getenv("PATH"));
		}else{
			/*value not exist, add new value into env*/
			if(setenv(cmd[1],cmd[3],0))
				fprintf(stderr,"Error in set env with error:%s\n",strerror(errno));
			//fprintf(stderr,"new Path 2: %s\n",getenv(cmd[1]));
		}
	}
}


void PWD(){
	char *pwd = malloc(100);
	getcwd(pwd,100);
	fprintf(stdout,"%s\n",pwd);
	free(pwd);
}

void splitPath(char *cmd[]){
	char path2[2014];
	char *token;

	int index = 0;
	char *paths = getenv("PATH");
	strcat(path2,paths);

	token = strtok(path2, ":");
	
	//copy all paths to string array
		while( token != NULL ) 
	  	{
	    
		 tokens[index]=token;
		 token = strtok(NULL, ":");
		 index++;
	 }


}

void HELP(){
	fprintf(stdout,"CSE320 SHELL:\n \
	cd					cd dicretory									\n \
	ls					show all file name under current directory		\n \
	set					set environ in format name = value				\n \
	pwd					show current directory							\n \
	echo					print string and expand environment variables	\n \
	help					print help meun									\n");
}

void handleCursor(char cmd[],char* originalPos,char last_char){


	
	if((&cmd[-1]>originalPos)||(&cmd[-2]>originalPos)){


	}else{
		if(cmd[0]=='A'&&cmd[-1]=='['&&(int)cmd[-2]==27){
			printf("UP\n" );
		}



	}



}










