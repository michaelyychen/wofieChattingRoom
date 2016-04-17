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
#include <sys/select.h>
#include "myHeader.h"

#define MAXLINE 1024

char cc = 0x1B;
char bb = 0x5B;
char username[20];
char host[20];
char port[20];
int clientfd;

int main (int argc, char ** argv) {
	



	if(argc<4){
		errorPrint();
		fprintf(stderr,"usage : %s <username> <host> <port> \n",argv[0]);
		exit(0);
	}
	username=argv[1];
	host=argv[2];
	port=argv[3];

	if((clientfd=open_clientfd(argv[2],argv[3])) < 0){
		errorPrint();
		fprintf(stderr,"Open client fd failed\n");
		exit(0);
	}

/*for multiIndexing*/
	fd_set read_set, ready_set;
	FD_ZERO(&read_set);
	FD_SET(STDIN_FILENO,&read_set);
	FD_SET(clientfd,&read_set);

	/*loop to see where input is from*/	
	while(1){
		ready_set = read_set;
		Select(clientfd+1,&ready_set);
		/*check for input from stdin*/
		if(FD_ISSET(STDIN_FILENO,&ready_set))
			stdinCommand();
		if(FD_ISSET(clientfd,&ready_set))
			serverCommand(clientfd);
		
	}
	
	Close(clientfd);
	exit(0);
}
void stdinCommand(){
	char buf[MAXLINE];
	if(!fgets(buf,MAXLINE,stdin))
		exit(0);
	if(!strcmp(buf,"\"help")){
		
	}else if(!strcmp(buf,"\"logout")){
		write(clientfd,"BYE\r\n\r\n",5);
	}else if(!strcmp(buf,"\"listu")){
		write(clientfd,"LISTU\r\n\r\n",5);
	}else if(!strcmp(buf,"\"time")){
		write(clientfd,"TIME\r\n\r\n",5);
	}else if(!strncmp(buf,"\"chat",5)){
		startChat();
	}


}

void serverCommand(int clientfd){

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

void Select(int n,fd_set *set){
	if(select(n,set,NULL,NULL,NULL)<0)
		fprintf(stderr,"Error on select\n");	
}

void HELP(){
	fprintf(stdout,"Client Usage:\n \
	./client [-hcv]		NAME SERVER_IP SERVER_PORT							\n \
	-h			Displays this help menu, and returns EXIT_SUCCESS.	\n \
	-c			Requests to server to create a new user				\n \
	-v			Verbose print all incoming and outgoing protocol verbs&content.			\n \
	NAME		This is the username to display when chatting.	\n \
	SERVER_IP	The IP Address of the server to connect to.		\n \
	SERVER_PORT	The port to connect to.	");
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

void errorPrint(){
	color("red");
	fprintf(stderr, "error: " );
	color("white");

}