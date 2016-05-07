#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/select.h>
#include <termios.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include "myHeader.h"
#include "sfwrite.h"


#define MAXLINE 1024
int socketFD;

char *buf = "temp";
char buffer[1024];
char fdS[20];
int fd;
int logFD;
pthread_mutex_t mut;
void sigInt_handler(int sigID){
   shutDown();
       
}


int main(int argc, char *argv[]) {

  pthread_mutex_init(&mut,NULL);
  signal(SIGINT,sigInt_handler);

  if(argc<3){

  fprintf(stderr,"Chat Usage:\n" );
  fprintf(stderr,
  "./chat UNIX_SOCKET_FD AUDIT_FILE_FD             \n \
   UNIX_SOCKET_FD The Number of FD to connect to. \n \
   AUDIT_FILE_FD The file descriptor of the audit file created in the cilent");
  exit(0);

  }

  strcpy(fdS,argv[1]);
  fd= stringToInt(argv[1]);
  logFD = stringToInt(argv[2]);
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
          if(!strcmp(buffer,"\n")){
            continue;
          }
          

      if((index=strchr(buffer,'\n'))!=NULL)
        *index = '\0';
      
      write(fd,buffer,1024);
      }
    
    if(FD_ISSET(fd,&ready_set)){
      read(fd,buffer,1024);
      if(!strncmp(buffer,"disconnect",10)){
         read(1,buffer,1);
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
  read(fd,buffer,sizeof(buffer));

  flock(logFD,LOCK_EX);
  FILE * logS = fdopen(logFD,"a+");
  sfwrite(&mut,logS,"%s",buffer);
  fflush(logS);
  flock(logFD,LOCK_UN);

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
