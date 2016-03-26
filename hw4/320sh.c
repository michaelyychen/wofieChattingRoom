#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "myHeader.h"
// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_ARG 128


int main (int argc, char ** argv, char **envp) {

  int finished = 0;
  char *prompt = "320sh> ";
  char cmd[MAX_INPUT];;
  int index = 0;



  while (!finished) {
    char *cursor;
    char last_char;
    int rv;
    int count;


    // Print the prompt
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

      rv = read(0, cursor, 1);
  	  
      last_char = *cursor;
      
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

		printf("job = %d\n",job);
		
		/*check to see if command is exit */
			if(!strcmp(argv[0],"exit")){		
				/*exit program*/
				write(1,"PROGRAM EXITING\n",17);
				exit(EXIT_SUCCESS);
			}
			else{
			/*find path of binary file*/
				char newPath[1028] = "";
				
				findPath(argv[0],newPath);
				printf("path is : %s\n",newPath);
				if(newPath != NULL){
					/*create a child and invoke function if path is not null*/
					if((pid = fork()) == 0){
						/*child process*/
						char *environ[]= {NULL};
						#ifdef d
							fprintf(stderr,"RUNNING : %s",cmd);
						#endif
						if(execve(newPath,argv,environ) < 0)
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
								fprintf(stderr,"Child terminated abnormally");
								return;
							}
						}else{
							fprintf(stderr,"ERROR ON WAITPID");
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
	int temp = index -1;
	str = argv[temp];	
	str = strtok(str, "\n");


	//background process condition
	if(*argv[--index]=='&'){
	argv[index] = NULL;
	return 0;
	}
	
	//foreground process
	return 1;

	

}

void findPath(char *path,char newPath[]){

  	char *temp;
	char *token;
	char *tokens[20];
 	char fullPath[1028]="";
	const char s[1] = ":";
	int index = 0;
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
	
	temp = getenv("PATH");
	token = strtok(temp, s);
	
	//copy all paths to string array
		while( token != NULL ) 
	  	{
		 tokens[index]=token;
		 token = strtok(NULL, s);
		 index++;
	   	}

		for(int i=0;i<index;i++){

		strcpy(fullPath,tokens[i]);
		strcat(fullPath,"/");
		strcat(fullPath,path);

			if(file_exist(fullPath)){	
				strcpy(newPath,fullPath);
				foundPath = 1;
				break;
			}
		
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













