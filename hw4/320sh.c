#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "myHeader.h"
// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

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
	  && (last_char ! = '\n');
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

  }

  return 0;
}

int parse(char buf[],char *argv[]){


  
   const char s[2] = " ";
   char *token;
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
	
	//background process condition
	if(*argv[--index]=='&'){
	argv[index] = NULL;
	return 0;
	}
	
	//foreground process
	return 1;

	

}

















