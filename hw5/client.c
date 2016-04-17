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
#include "myHeader.h"

#define MAXLINE 1024

char cc = 0x1B;
char bb = 0x5B;

int main (int argc, char ** argv) {
	/*
	//int clientfd;
	//char *host,*port;


	if(argc!=3){
		fprintf(stderr,"usage : %s<host> <port> \n",argv[0]);
		exit(0);
	}

	//host=argv[1];
	//port=argv[2];
	*/
	color("yellow");

	fprintf(stdout,"usage : %s<host> <port> \n",argv[0]);


	//Close(clientfd);
	exit(0);
}

int open_clientfd(char * hostname, char * port){

	struct addrinfo hints,*listPointer,*p;
	int clientfd;

	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;
	hints.ai_flags |= AI_ADDRCONFIG;
	Getaddrinfo(hostname,port,&hints,&listPointer);
	for(p=listPointer; p ; p=p->ai_next) {
		if((clientfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))<0)
			continue;

		if(connect(clientfd,p->ai_addr,p->ai_addrlen)!=-1){
			break;
		}

		Close(clientfd);

	}

		freeaddrinfo(listPointer);
		if(!p)		//if failed
			return -1;
		else
			return clientfd;

}


int Getaddrinfo(const char* host,
				const char*service,
				const struct addrinfo *hints,
				 struct addrinfo **result){

	int resultt;
	resultt = getaddrinfo(host,service,hints, result);
	if(resultt != 0)
		fprintf(stderr,"Getaddrinfo with error: %s\n",gai_strerror(resultt));
	return resultt;

}


int Close(int clientfd){

	int result;
	result = close(clientfd);
	if(result < 0)
		fprintf(stderr,"Close with error: %s\n",strerror(errno));
	return result;
}


void HELP(){
	fprintf(stdout,"Client Usage:\n \
	./client [-hcv]		NAME SERVER_IP SERVER_PORT							\n \
	-h					Displays this help menu, and returns EXIT_SUCCESS.	\n \
	-c					Requests to server to create a new user				\n \
	-v					Verbose print all incoming and outgoing protocol verbs&content.			\n \
	NAME				This is the username to display when chatting.	\n \
	SERVER_IP			The IP Address of the server to connect to.		\n \
	SERVER_PORT			The port to connect to.	");
}



void color(char* color){

	char cc = 0x1B;
	char bb = 0x5B;
	char mm = 109;
	
    write(1,&cc,1);  
    write(1,&bb,1);
   
    if(strcmp(color,"red")==0){
    	write(1,"31",2);
    }else if (strcmp(color,"green")==0){
		write(1,"32",2);
    }else if(strcmp(color,"yellow")==0){
		write(1,"33",2);
    }else if(strcmp(color,"blue")==0){
		write(1,"34",2);
    }else if(strcmp(color,"magenta")==0){
		write(1,"35",2);
    }else if(strcmp(color,"cyan")==0){
		write(1,"36",2);
    }else {
    	write(1,"37",2);
    }
  
    write(1,&mm,1);
}