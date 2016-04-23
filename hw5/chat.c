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

  read(fd,buffer,1024);
  printf("%s\n",buffer );

    int  number;

    while(number!=10){
    printf("Type in a number \n");
    scanf("%d", &number);
    printf("The number you typed was %d\n", number);
}

  close(fd);
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