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

int main (int argc, char ** argv) {
	
	int listenfd,connfd;
	socklen_t clientlen;
	struct sockaddr_storage clientadr;
	char client_hostname[MAXLINE], client_port[MAXLINE];
	
	if(argc < 2){
		fprintf(stderr,"Missing argument\n");
		exit(0);
		}
	
	if((listenfd = open_listenfd(argv[1]) < 0)){
		fprintf(stderr,"Open listen fd failed\n");
		exit(0);
	}
	
	while(1){
		clientlen = sizeof(struct sockaddr_storage);
		connfd = accept(listenfd, (SA*)&clientaddr,&clientlen);
		if(connfd>0){
		 fprintf(stdout,"connected!!!!!!!!!");
		 exit(0);
		 }
	}
	
	
	exit(0);
}

int open_listenfd(char * port){
	struct addrinfo  hints, *list, *pt;
	int listenfd, optval=1;
	
	/*create a list of potential server addresses*/
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	hints.ai_flags |= AI_NUMERICSERV;
	Getaddrinfo(Null, port, &hints,&listp);
	
	/*loop to find a ip to bind*/
	for(p=list;p;p->ai.next){
		/*try to create a socket */
		if((listenfd = socket(p->ai_family,p->ai_socketype,p->ai_protocol)) < 0)
			continue; 
			
		Setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int));
		
		/*bind*/
		if(bind(listenfd,p_>ai_addr,p_>ai_addrlen)==0)
			break;
		Close(listenfd);
	}
	
	/*free space*/
	freeaddrinfo(listp);
	if(!p)
	return -1;
	
	if(listen(listenfd,1024) < 0){
		CLose(listenfd);
		return -1;
	}
	
	return listenfd;

}


int Getaddrinfo(const char* host,
				const char*service,
				const struct addrinfo *hints,
				 struct addrinfo **result){

	int result;
	result = getaddrinfo(host,service,hints, result);
	if(result < 0)
		fprintf(stderr,"Getaddrinfo: %s with error: %s\n",pathname,strerror(errno));
	return result;

}

int Close(int clientfd){

	int result;
	result = close(clientfd);
	if(result < 0)
		fprintf(stderr,"Close: %s with error: %s\n",pathname,strerror(errno));
	return result;
}

int Freeaddrinfo(const struct addrinfo *listPointer){
	int result;
	result = freeaddrinfo(listPointer);
	if(result < 0)
		fprintf(stderr,"Freeaddrinfo: %s with error: %s\n",pathname,strerror(errno));
	return result;
}
