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
#define nameTaken 0
#define notAvailable 1
#define badPassword 2
#define serverError 100

struct User{
	time_t loginTime;
	int clientSock;
	char name[1000];
	struct in_addr addr;
	struct User *next;
};
typedef struct User User;

struct accountList{
	char name[1000];
	char pwd[1000];
	struct accountList *next;
};
typedef struct accountList accountList;

struct communicatePair{
	int fd;
	struct in_addr addr;
};
typedef struct communicatePair communicatePair;
/*global variables*/
int verbose = 0;
int userCount = 0;
char *welcomeMessage;
char port[20];
User *userHead = NULL;
accountList *accHead = NULL;

int main (int argc, char ** argv){

	/*set debug color*/
	color("red",2);
	color("green",1);
	int listenfd;

	time_t current_time = time(0);
	getTime(current_time);

	/*check for arguments*/
	if(argc < 3){
		fprintf(stderr,"Missing argument\n");
		exit(0);
		}

	welcomeMessage = argv[2];
	strcpy(port,argv[1]);
	/*initilize account link list*/

	/*check for optional argument*/
	int opt = 0;
	while((opt = getopt(argc,argv,"hv")) != -1){
		if(opt == 'h'){
			HELP();
			exit(EXIT_SUCCESS);
		}
		else if(opt == 'v'){
			verbose = 1;
		}

	}

	if((listenfd = open_listenfd(port)) < 0){
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

void *loginThread(void *Cpair){
	//printf("login thread\n");
	int log = 1;
	/*read from client*/
	communicatePair *pair = Cpair;
	char buf[MAXLINE];
	Read(pair->fd,buf,MAXLINE);
	/*compare protocol*/
	if(!strncmp(buf,"WOLFIE \r\n\r\n",11)){
		writeV(pair->fd,"EIFLOW \r\n\r\n",11);
	}else{
		/*login fail*/
		log =0;
	}

	Read(pair->fd,buf,MAXLINE);
	char name1[50];
    char *token;
    token = strtok(buf, " ");
	if(!strcmp(token,"IAM")){
	     token = strtok(NULL, " ");
		 strcpy(name1,token);
		 token = strtok(NULL, " ");
		 if(strcmp(token,"\r\n\r\n")){
			 /*login fail*/
			 log=0;
		 }
	 }else{
		 log=0;
		 /*login fail*/
	 }
	 /*check if user already login in*/
	 if(!checkLogin(name1)){
		 handleError(nameTaken,pair->fd);
		 log = 0;
	 }

   if(log){
	   printf("login sucess! %s\n",name1);
	   char msg[MAXLINE];
	   strcpy(msg,"HI ");
	   strcat(msg,name1);
	   strcat(msg," \r\n\r\n");
	   writeV(pair->fd,msg,(8+strlen(name1)));
	   addUser(name1,(void*)pair);
	   strcpy(msg,"MOTD ");
	   strcat(msg,welcomeMessage);
	   strcat(msg," \r\n\r\n");
	   writeV(pair->fd,msg,(strlen(welcomeMessage)+10));

   }else
   		printf("login fail!\n");

	return NULL;
}

void addUser(char *name, void *pair){
	/*find last user*/
	User *temp = userHead;
	if(temp != NULL){
		while(temp->next!=NULL)
			temp = temp->next;
	}
	communicatePair *p = pair;
	User newUser;
	strcpy(newUser.name,name);
	newUser.clientSock = p->fd;
	newUser.addr = p->addr;
	newUser.next = NULL;
	newUser.loginTime = time(0);

	if(temp != NULL)
		temp->next = &newUser;
	else
		temp = &newUser;
}

void handleError(int error_code,int fd){
	if(error_code==nameTaken){
		writeV(fd,"ERR 00 USER NAME TAKEN \r\n\r\n",30);
		writeV(fd,"BYE \r\n\r\n",10);
		/*read bye from client*/
		char *temp = malloc(8);
		Read(fd,temp,8);
		if(!strcmp(temp,"BYE \r\n\r\n")){
			Close(fd);
		}

	}else if(error_code==notAvailable){

	}else if(error_code==badPassword){

	}else{

	}

}
int writeV(int fd, char *s, int byte){
	int result = write(fd,s,byte);
	if(verbose)
		printf("%s\n",s);
	if(result == -1){
		fprintf(stderr,"Error on writing to FD: %d\n Error: %s\n",fd,strerror(errno));
	}
	return result;
}

int Close(int fd){
	int result;
	if((result = close(fd))== -1)
		fprintf(stderr,"Error on closing file, file descriptor: %d\n	\
		Error: %s\n",fd,strerror(errno));
	return result;
}

int checkLogin(char *name){
	/*if user already login in, return 0*/
	int currentlyIn = 1;
	User *temp = userHead;
	while(temp!=NULL){
		if(!strcmp(temp->name,name))
			currentlyIn = 0;
		temp = temp->next;
	}
	return currentlyIn;
}

void stdinCommand(){
	char buf[MAXLINE];
	if(scanf("%s",buf)<0)
		fprintf(stderr,"Error reading standard input\n");

	if(!strcmp(buf,"/users")){
		users();
		printf("users\n");
	}else if(!strcmp(buf,"/help")){
		HELP();
		printf("call help\n");
	}else if(!strcmp(buf,"shutdown")){
		shutDown();
		printf("shutdown\n");
	}
}
void users(){

}
void shutDown(){

}
void clientCommand(int listenfd){

/*input from listenfd, accept input*/
	int connfd;
	struct sockaddr_storage clientaddr;
	socklen_t clientlen = sizeof(struct sockaddr_storage);

	connfd = Accept(listenfd, (struct sockaddr*)&clientaddr,&clientlen);

	pthread_t tid;

	/*spawn a login thread*/
	communicatePair pair;
	pair.fd = connfd;
	pair.addr = ((struct sockaddr_in*)&clientaddr)->sin_addr;
	//printf("fd: %d addr: %d\n",pair.fd,pair.addr.s_addr);
	if(pthread_create(&tid,NULL,loginThread,&pair)){
		fprintf(stderr,"Error on create thread\n");
	}
	/*wait for login thread to determine*/
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

void HELP(){
	fprintf(stdout,"Client Usage:\n \
	./server [-hv]		SERVER_PORT MOTD							\n \
	-h					Displays this help menu, and returns EXIT_SUCCESS.	\n \
	-v					Verbose print all incoming and outgoing protocol verbs&content.			\n \
	/users				Print a list of user currently logged in	\n \
	/help				Print this list of commands		\n \
	/shutdown			Close all socket, files and free heap memory then terminate\n");
}

ssize_t Read(int fd, void *buf,size_t count){
	ssize_t result = read(fd,buf,count);
	if(result == -1)
		printf("Error on read: %s\n",strerror(errno));
	return result;
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

void color(char* color,int fd){

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
