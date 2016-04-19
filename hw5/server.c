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
#include <time.h>
#include <pthread.h>
#include "myHeader.h"

#define _GUN_SOURCE
#define MAXLINE 1024


typedef struct User{
	time_t loginTime;
	int clientSock;
	char name[20];
	struct in_addr addr;
	struct User *next;
}UserHead;

typedef struct ThreadList{
	pthread_t tid;
	struct ThreadList *next;
}threadHead;

int main (int argc, char ** argv){
	Color("red",2);
	Color("green",1);
	int listenfd;

	time_t current_time = time(0);
	getTime(current_time);

	if(argc < 3){
		fprintf(stderr,"Missing argument\n");
		exit(0);
		}

	int opt = 0;
	while((opt = getopt(argc,argv,"hv")) != -1){
		if(opt == 'h'){
			HELP();
			exit(EXIT_SUCCESS);
		}
		else if(opt == 'v'){

		}

	}

	if((listenfd = open_listenfd(argv[1])) < 0){
		fprintf(stderr,"Open listen fd failed\n");
		exit(0);
	}


	fprintf(stdout,"Currently listening on port %s\n",argv[1]);


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

void *loginThread(void *vargp){
	printf("thread ok!\n");
	return NULL;
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

	connfd = Accept(listenfd, (struct sockaddr*)&clientaddr,&clientlen);
	pthread_t tid;
	if(pthread_create(&tid,NULL,loginThread,NULL)){
		fprintf(stderr,"Error on create thread\n");
	}
	Pthread_join(tid,NULL);
	fprintf(stdout,"connected address is : %d \n",
		((struct sockaddr_in*)&clientaddr)->sin_addr.s_addr);
	if(connfd>0){
	 fprintf(stdout,"connected!!!!!!!!\n");
	 exit(0);
	 }


}

void getTime(time_t current_time){

	struct tm* timeinfo;
	timeinfo = localtime(&current_time);
	fprintf(stdout,"time in second is [%d %d %d %d:%d:%d]\n",
		timeinfo->tm_mday, timeinfo->tm_mon + 1,
		timeinfo->tm_year + 1900, timeinfo->tm_hour,
		timeinfo->tm_min, timeinfo->tm_sec);
}

void Select(int n,fd_set *set){
	if(select(n,set,NULL,NULL,NULL)<0)
		fprintf(stderr,"Error on select\n");
}

int Accept(int socket, struct sockaddr *addr, socklen_t *socklen){
	int result = accept(socket, addr, socklen);
	if(result == -1){

		fprintf(stderr,"Error in accept connection: %s\n",strerror(errno));

	}
	return result;
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
	./server [-hv]		SERVER_PORT MOTD							\n \
	-h					Displays this help menu, and returns EXIT_SUCCESS.	\n \
	-v					Verbose print all incoming and outgoing protocol verbs&content.			\n \
	/users				Print a list of user currently logged in	\n \
	/help				Print this list of commands		\n \
	/shutdown			Close all socket, files and free heap memory then terminate\n");
}



int Pthread_join(pthread_t tid, void **thread_return){
	int result = pthread_join(tid,thread_return);
	if(result!=0){
		fprintf(stderr,"Error join thread\n");
	}

	return result;
}

int Pthread_detach(pthread_t tid){
	int result = pthread_detach(tid);
	if(result!=0){
		fprintf(stderr,"Error detach thread\n");
	}
	return result;
}

void Color(char* color,int fd){

	char cc = 0x1B;
	char bb = 0x5B;
	char mm = 109;

    write(fd,&cc,1);
    write(fd,&bb,1);


    if(strcmp(color,"red")==0){
    	write(fd,"31",2);
    }else if (strcmp(color,"green")==0){
		write(fd,"32",2);
    }else if(strcmp(color,"yellow")==0){
		write(fd,"33",2);
    }else if(strcmp(color,"blue")==0){
		write(fd,"34",2);
    }else if(strcmp(color,"magenta")==0){
		write(fd,"35",2);
    }else if(strcmp(color,"cyan")==0){
		write(fd,"36",2);
    }else {
    	write(fd,"37",2);
    }

    write(fd,&mm,1);
}
