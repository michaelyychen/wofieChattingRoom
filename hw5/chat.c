#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/select.h>
#include "myHeader.h"

#define MAXLINE 1024
int socketFD;

char *buf = "temp";

int main(int argc, char *argv[]) {

  if(argc<2){


  fprintf(stderr,"Chat Usage:\n" );
  fprintf(stderr,
  "./chat UNIX_SOCKET_FD             \n \
   UNIX_SOCKET_FD The Number of FD to connect to. \n");
  exit(0);

  }

  char buffer[1024];
  int fd = stringToInt(argv[1]);

  fd_set read_set,ready_set;

  char * index;

  FD_ZERO(&read_set);
  FD_SET(STDIN_FILENO,&read_set);
  FD_SET(fd,&read_set);
  //childList *temp = childHead;
  /*loop to see where input is from*/
  while(1){
    ready_set=read_set;

    select(fd+1,&ready_set,NULL,NULL,NULL);
    
    
    /*check for input from stdin*/
    if(FD_ISSET(STDIN_FILENO,&ready_set)){
      memset(buffer,0,MAXLINE);
     
       fgets(buffer,1024,stdin);

          if(!strncmp(buffer,"/close",6)){
            close(fd);
            exit(EXIT_SUCCESS);
          }

      if((index=strchr(buffer,'\n'))!=NULL)
        *index = '\0';
      
      write(fd,buffer,1024);
      }
    
    if(FD_ISSET(fd,&ready_set)){
      read(fd,buffer,1024);
      printf("%s\n",buffer );
    }
      
   

   
    
    }

  
  return 0;
}

int stringToInt(char* str){
  int result=0;
  int i;
  int stringLen = strlen(str);

  for(i=0; i<stringLen; i++){

  result = result * 10 + ( str[i] - '0' );

  }
  return result;
}