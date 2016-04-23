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

  int fd = stringToInt(argv[1]);
  printf("arg 1%s\n",argv[1] );
  printf("arg 1%d\n",fd );

  write(fd,"avbdsdfdsf",10);

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