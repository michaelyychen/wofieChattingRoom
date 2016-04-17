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

int main (int argc, char ** argv){
	
	int listenfd;

	if(argc < 2){
		fprintf(stderr,"Missing argument\n");
		exit(0);
		}
	
	if((listenfd = open_listenfd(argv[1])) < 0){
		fprintf(stderr,"Open listen fd failed\n");
		exit(0);
	}

/*for multiIndexing*/
	fd_set read_set, ready_set;
	FD_ZERO(&read_set);
	FD_SET(STDIN_FILENO,&read_set);
	FD_SET(listenfd,&read_set);

	/*loop to see where input is from*/	
	while(1){
		ready_set = read_set;
		Select(listenfd+1,&ready_set);
		/*check for input from stdin*/
		if(FD_ISSET(STDIN_FILENO,&ready_set))
			stdinCommand();
		if(FD_ISSET(listenfd,&ready_set))
			clientCommand(listenfd);
		
	}
	
	
	exit(0);
}
void stdinCommand(){
	char buf[MAXLINE];
	if(!fgets(buf,MAXLINE,stdin))
		exit(0);
	printf("%s\n",buf);
}

void clientCommand(int listenfd){
	/*spawn a login thread*/

	int connfd;
	struct sockaddr_storage clientaddr;
	socklen_t clientlen = sizeof(struct sockaddr_storage);

	connfd = accept(listenfd, (struct sockaddr*)&clientaddr,&clientlen);
	fd_set read_set;
	FD_ZERO(&read_set);
	FD_SET(STDIN_FILENO,&read_set);
	FD_SET(connfd,&read_set);
	Select(connfd+1,&read_set);
	if(FD_ISSET(connfd,&read_set)){

		fprintf(stdout, "asdasd\n" );
	}

}

void Select(int n,fd_set *set){
	if(select(n,set,NULL,NULL,NULL)<0)
		fprintf(stderr,"Error on select\n");	
}


int open_listenfd(char * port){
	struct addrinfo  hints, *list, *pt;
	int listenfd;
	
	/*create a list of potential server addresses*/
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	hints.ai_flags |= AI_NUMERICSERV;
	Getaddrinfo(NULL, port, &hints,&list);
	
	/*loop to find a ip to bind*/
	for(pt=list;pt;pt=pt->ai_next){
		/*try to create a socket */
		if((listenfd = socket(pt->ai_family,pt->ai_socktype,pt->ai_protocol)) < 0)
			continue; 
			
		//setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int));
		
		/*bind*/
		if(bind(listenfd,pt->ai_addr,pt->ai_addrlen)==0)
			break;
		Close(listenfd);
	}
	
	/*free space*/
	freeaddrinfo(list);
	if(!pt)
	return -1;
	
	if(listen(listenfd,1024) < 0){
		Close(listenfd);
		return -1;
	}
	
	return listenfd;

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
