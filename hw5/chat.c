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
char buffer[1024];
char fdS[20];
int fd;
void sigInt_handler(int sigID){
   shutDown();
       
}


int main(int argc, char *argv[]) {


  signal(SIGINT,sigInt_handler);

  if(argc<2){

  fprintf(stderr,"Chat Usage:\n" );
  fprintf(stderr,
  "./chat UNIX_SOCKET_FD             \n \
   UNIX_SOCKET_FD The Number of FD to connect to. \n");
  exit(0);

  }

  strcpy(fdS,argv[1]);
  fd= stringToInt(argv[1]);

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
            shutDown();
          }

      if((index=strchr(buffer,'\n'))!=NULL)
        *index = '\0';
      
      write(fd,buffer,1024);
      }
    
    if(FD_ISSET(fd,&ready_set)){
      read(fd,buffer,1024);
      if(!strncmp(buffer,"disconnect",10)){
         fgets(buffer,1024,stdin);
         shutDown();
      }
      printf("%s\n",buffer );
    }
      
   

   
    
    }

  
  return 0;
}
void shutDown(){

  memset(buffer,0,MAXLINE);
  strcat(buffer,"remove");
  strcat(buffer," ");
  strcat(buffer,fdS);
  strcat(buffer," ");
  write(fd,buffer,1024);
  exit(EXIT_SUCCESS);

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