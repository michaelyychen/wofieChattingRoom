#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "myHeader.h"
// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_ARG 128

int 
main (int argc, char ** argv, char **envp) {

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
		//pid_t pid;			 /*new process id*/
		
		strcpy(buf, cmd);

		/*parse command line*/
		job = parse(buf,argv);
		int i = 0;

		printf("job = %d\n",job);
		while(argv[i]!=NULL){
			fprintf(stdout,"argument %d is : %s\n",i,argv[i]);
			i++;
		}

		/*check to see if command if build in*/
		if(!buildIn(argv[0])){
			/*build in function, handle them*/
			
			if(!strcmp(argv[0],"exit")){
				
				/*exit program*/
				write(1,"PROGRAM EXITING\n",17);

				exit(EXIT_SUCCESS);
			}
			if(strcmp(argv[0],"pwd")){}
		
		}else{
		/*not build in commnad*/
			
		}
			
		
}
	
int buildIn(char * cmd){
	return 0;
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

















