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
  char buffer[1024];
  int fd = stringToInt(argv[1]);

   while(1){


    read(fd,buffer,1024);
    printf("%s\n",buffer );

    memset(buffer,0,MAXLINE);
    scanf("%s", buffer);

    if(!strncmp(buffer,"/close",6)){
      close(fd);
      exit(EXIT_SUCCESS);
    }

    write(fd,buffer,1024);
    
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