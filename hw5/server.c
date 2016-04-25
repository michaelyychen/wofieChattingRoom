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
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>
#include <time.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
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
	unsigned char pwd[SHA256_DIGEST_LENGTH];
	unsigned char salt[5];
	struct accountList *next;
};

typedef struct accountList accountList;

struct communicatePair{
	int fd;
	struct in_addr addr;
};

typedef struct communicatePair communicatePair;

/*global variables*/
int listenfd;
int acctFd = 0;
int verbose = 0;
int userCount = 0;
char *welcomeMessage;
char port[20];
User *userHead = NULL;
accountList *accHead = NULL;

int main (int argc, char ** argv){


	signal(SIGINT,sigInt_handler);

	/*check for arguments*/
	if(argc < 3){
		color("red",1);
		fprintf(stderr,"Missing argument\n");
		color("white",1);
		exit(0);
		}

	welcomeMessage = argv[2];
	strcpy(port,argv[1]);
	

	/*check if account list if provided*/
	if(argc >=4){
	
		int i = 3;
		while(i<argc){
			
			acctFd = Open(argv[i],O_RDWR|O_APPEND,S_IWUSR|S_IRUSR|S_IXUSR);
			if(acctFd>0){
				/*initilize account link list*/
				printf("file exit\n");
				/*
				struct stat st;
				stat(argv[i],&st);
				int size = st.st_size;
				accountList *acctTemp = accHead;
				char buf[size];
				if(Read(acctFd,buf,size)>0){
					
					char *temp = buf;
					while(size>0){
						accountList *acct = malloc(sizeof(accountList));
						memcpy(acct->name,temp,1000);
						temp += 1005;
						memcpy(acct->pwd,temp,32);
						temp += 37;
						memcpy(acct->salt,temp,5);
						size -= 1052;
						acct->next = NULL;
						if(accHead==NULL){
							accHead = acct;
							acctTemp = accHead;
						}else{
							acctTemp->next = acct;
							acctTemp = acctTemp->next;
						}
					}

				}*/
				accountList *acctTemp = accHead;
				char buf[1000];
				
				while(Read(acctFd,buf,1000)>0){
					accountList *acct = malloc(sizeof(accountList));
					memcpy(acct->name,buf,1000);
					Read(acctFd,buf,5);
					Read(acctFd,acct->pwd,32);
					Read(acctFd,buf,5);
					Read(acctFd,acct->salt,5);
					Read(acctFd,buf,5);
					acct->next = NULL;
					if(accHead==NULL){
							accHead = acct;
							acctTemp = accHead;
						}else{
							acctTemp->next = acct;
							acctTemp = acctTemp->next;
						}
				}	
				break;

			}

			i++;
		}
	}
		

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
		color("red",1);
		fprintf(stderr,"Open listen fd failed\n");
		color("red",1);
		exit(0);
	}

	color("green",1);
	fprintf(stdout,"Currently listening on port %s\n",port);
	color("white",1);

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

void addAcct(char *name, char *pwd){

	accountList *acct = malloc(sizeof(accountList));
	strcpy(acct->name,name);

	if(!RAND_bytes(acct->salt,5)){
		color("red",2);
		fprintf(stderr,"Error on generating random bytes\n");
		color("white",2);
	}

	getHash((void*)acct,pwd);

	acct->next = NULL;

	accountList *temp = accHead;

	if(temp!=NULL){
		while(temp->next!=NULL){
			temp = temp->next;
		}
	}

	if(accHead==NULL){
		accHead = acct;
		/*open account file and add to it*/
		acctFd = Open("./Account.txt",O_RDWR|O_TRUNC|O_CREAT,S_IWUSR|S_IRUSR|S_IXUSR);
		writeV(acctFd,acct->name,sizeof(acct->name));
		writeV(acctFd,"\n\n\n\n\n",5);
		writeV(acctFd,(char*)acct->pwd,SHA256_DIGEST_LENGTH);
		writeV(acctFd,"\n\n\n\n\n",5);
		writeV(acctFd,(char*)acct->salt,5);
		writeV(acctFd,"\n\n\n\n\n",5);

	}
	else{
		temp->next = acct;
		writeV(acctFd,acct->name,sizeof(acct->name));
		writeV(acctFd,"\n\n\n\n\n",5);
		writeV(acctFd,(char*)acct->pwd,SHA256_DIGEST_LENGTH);
		writeV(acctFd,"\n\n\n\n\n",5);
		writeV(acctFd,(char*)acct->salt,5);
		writeV(acctFd,"\n\n\n\n\n",5);
	}

	
}

void sigInt_handler(int sigID){
	shutDown();
}

int checkPwd(char * pwd){

	int result = 1,upcase =0,symbol = 0,number = 0;
	char *temp = pwd;

	int count = strlen(pwd);

	if(count < 5)
		return 0;

	while(count>0){
		printf("%c",*temp);
		if(*temp>='0' && *temp<='9')
			number = 1;
		else if(*temp>='A' && *temp<='Z')
			upcase = 1;
		else if((*temp>='!' && *temp<='/') || (*temp>=':' && *temp<='@')
		 || (*temp>='[' && *temp<='`') || (*temp>='{' && *temp<= '~'))
			symbol = 1;
	
		temp++;
		count--;

	}

	result = upcase & symbol;
	result = result & number;
	return result;

}

int Open(const char* pathname, int flags, mode_t mode){
	int result;
	result = open(pathname,flags,mode);
	if(result < 0)
		fprintf(stderr,"Open file: %s with error: %s\n",pathname,strerror(errno));
	return result;
}

void getHash(void *acct, char *pwd){

	accountList *acc = (accountList*)acct;

	SHA256_CTX temp;

	if(!SHA256_Init(&temp)){
		color("red",2);
		fprintf(stderr,"Error on hashing password: INIT\n");
		color("white",2);
	}
	if(!SHA256_Update(&temp,pwd,strlen(pwd))){
		color("red",2);
		fprintf(stderr,"Error on hashing password: Update\n");
		color("white",2);
	}
	
	if(!SHA256_Update(&temp,acc->salt,5)){
		color("red",2);
		fprintf(stderr,"Error on hashing password: Update with Salt\n");
		color("white",2);
	}
	if(!SHA256_Final(acc->pwd,&temp)){
		color("red",2);
		fprintf(stderr,"Error on hashing password: Final\n");
		color("white",2);
	}

}

int compareHash(char *name, char *pwd){

	accountList *acc = accHead;

	if(acc==NULL)
		return 0;

	while(strcmp(acc->name,name)){
		acc = acc->next;
		if(acc==NULL)
			return 0;
	}

	unsigned char compare[SHA256_DIGEST_LENGTH];
	SHA256_CTX temp;


	if(!SHA256_Init(&temp)){
		color("red",2);
		fprintf(stderr,"Error on hashing password: INIT\n");
		color("white",2);
	}
	if(!SHA256_Update(&temp,pwd,strlen(pwd))){
		color("red",2);
		fprintf(stderr,"Error on hashing password: Update\n");
		color("white",2);
	}
	
	if(!SHA256_Update(&temp,acc->salt,5)){
		color("red",2);
		fprintf(stderr,"Error on hashing password: Update with Salt\n");
		color("white",2);
	}
	if(!SHA256_Final(compare,&temp)){
		color("red",2);
		fprintf(stderr,"Error on hashing password: Final\n");
		color("white",2);
	}

	if(!memcmp(acc->pwd,compare,32))
		return 1;

	return 0;

}

void *loginThread(void *Cpair){
	printf("---------login thread-------\n");

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
	char name1[1000];
    char *token;
    token = strtok(buf, " ");
	if(!strcmp(token,"IAM")){
		/*existing user login process*/
		
	     token = strtok(NULL, " ");
		 strcpy(name1,token);
		 token = strtok(NULL, " ");
		 if(strcmp(token,"\r\n\r\n")){
			 /*login fail*/
			 color("red",1);
			 fprintf(stderr,"client did not send rnrn\n");
			 color("white",1);
			 log=0;
		 }

		 log = existUser(Cpair,name1);
		 

	 }else if(!strcmp(token,"IAMNEW")){
	 	/*new user login process*/
	 	 token = strtok(NULL, " ");
		 strcpy(name1,token);
		 token = strtok(NULL, " ");
		 if(strcmp(token,"\r\n\r\n")){
			 /*login fail*/
			 color("red",1);
			 fprintf(stderr,"client did not send rnrn\n");
			 color("white",1);
			 log=0;
		 }

	 	 log = newUser(Cpair, name1);
	 	

	 }else{
		 color("red",1);
		 fprintf(stderr,"client did not send IAM\n");
		 color("white",1);
		 log=0;
		 /*login fail*/
	 }
	 

   if(log){

	   User* temp = (User*)addUser(name1,pair);
	   printf("login sucess! %s\n",name1);
	  
	   /*create communication thread*/
	   pthread_t tid;
	   if(pthread_create(&tid,NULL,talkThread,temp)){
   		color("red",1);
   		fprintf(stderr,"Error on create thread\n");
   		color("white",1);
   		}

   }else{
	   color("red",1);
   		fprintf(stderr,"login fail!\n");
		color("white",1);
	}
	return NULL;
}

int existUser(void *Cpair, char *name){

	communicatePair *pair = Cpair;
	/*check if user already login in*/
	int loginS;
		 if((loginS = checkLogin(name,1))<0){
		 	if(loginS == -1){
			 color("red",1);
			 fprintf(stderr,"same username\n");
			 color("white",1);
			 handleError(nameTaken,pair->fd);
			 return 0;
			}else if(loginS == -1){
				color("red",1);
				 fprintf(stderr,"same username\n");
				 color("white",1);
				 handleError(notAvailable,pair->fd);
				 return 0;
			}
		 }
			/*return message to client*/

		   char msg[MAXLINE];
		   strcpy(msg,"AUTH ");
		   strcat(msg,name);
		   strcat(msg," \r\n\r\n");
		   writeV(pair->fd,msg,MAXLINE);

		   /*read password from user*/
		   Read(pair->fd,msg,MAXLINE);
		   char *token = strtok(msg," ");
		   if(!strcmp(token,"PASS")){
		   		token = strtok(NULL," ");
		   		if(compareHash(name,token)){
		   		   strcpy(msg,"SSAP \r\n\r\n");
	   			   strcat(msg,"HI ");
				   strcat(msg,name);
				   strcat(msg," \r\n\r\n");
				   strcat(msg,"MOTD ");
				   strcat(msg,welcomeMessage);
				   strcat(msg," \r\n\r\n");
				   writeV(pair->fd,msg,MAXLINE);
		   		}else{
		   			handleError(badPassword,pair->fd);
		   			return 0;
		   		}
		   }else{
		   		fprintf(stderr,"Verb PASS is not received\n");
		   		return 0;
		   }
		  
		   
		   return 1;

}

int newUser(void *Cpair, char *name){

	communicatePair *pair = Cpair;

	char buf[MAXLINE];
	if(checkLogin(name,0)==0){
			 color("red",1);
			 fprintf(stderr,"same username\n");
			 color("white",1);
			 handleError(nameTaken,pair->fd);
			 return 0;
		 }
	/*say hi*/
	strcpy(buf,"HINEW ");
	strcat(buf,name);
	strcat(buf," \r\n\r\n");
	writeV(pair->fd,buf,MAXLINE);

	/*get password from user*/
	memset(buf,0,MAXLINE);
	Read(pair->fd,buf,MAXLINE);
	char *pwd = strtok(buf," ");
	pwd = strtok(NULL," ");
	if(checkPwd(pwd)){
		/*good password*/
		addAcct(name,pwd);
		memset(buf,0,MAXLINE);
		strcpy(buf,"SSAPWEN");
		strcat(buf," \r\n\r\n");
		strcat(buf,"HI ");
		strcat(buf,name);
		strcat(buf," \r\n\r\n");
		strcat(buf,"MOTD ");
		strcat(buf,welcomeMessage);
		strcat(buf," \r\n\r\n");
		writeV(pair->fd,buf,MAXLINE);
		printf("%s",buf);
		return 1;

	}else{
		/*handle bad password*/
		handleError(badPassword,pair->fd);
		return 0;
	}

}

void* talkThread(void* vargp){
	/*detach thread*/
	pthread_detach(pthread_self());
	User *user = (User*)vargp;
	/*naming the thread
	char *myName = "talk:";
	char fullName[10];
	strcpy(fullName,myName);
	strcat(fullName,((char*)&pair.fd));
	pthread_setname_np(pthread_self,fullName);
	*/

	/*loop to see where input is from*/
	char buf[MAXLINE];

	while(1){
		memset(buf,0,MAXLINE);
		Read(user->clientSock,buf,MAXLINE);
		if(!strcmp(buf,"TIME \r\n\r\n")){
			printf("process command TIME\n");
			int result = getTime(user->loginTime);
			memset(buf,0,MAXLINE);
			int temp = result, len = 2;
			while(temp%10>0){
				len++;
				temp = temp/10;
			}
			char timeBuf[len];
			sprintf(timeBuf,"%d",result);
			printf("time is %d\n",result);
			//intToS(timeBuf,result);
			strcpy(buf,"EMIT ");
			strcat(buf,timeBuf);
			strcat(buf," \r\n\r\n");
			writeV(user->clientSock,buf,MAXLINE);
		}
		else if(!strcmp(buf,"LISTU \r\n\r\n")){
			printf("process command LISTU\n");
			memset(buf,0,MAXLINE);
			strcpy(buf,"UTSIL ");
			User *temp = userHead;
			while(temp!=NULL){
				strcat(buf,temp->name);
				if(temp->next != NULL)
					strcat(buf," \r\n ");
				temp = temp->next;
			}
			strcat(buf," \r\n\r\n");
			printf("%s",buf);
			writeV(user->clientSock,buf,MAXLINE);
		}
		else if(!strcmp(buf,"BYE \r\n\r\n")){

			memset(buf,0,MAXLINE);
			strcpy(buf,"BYE \r\n\r\n");
			writeV(user->clientSock,buf,8);
			removeUser(user->clientSock);
			break;

		}else if(!strncmp(buf,"MSG",3)){

			char *nameTo,*nameFrom;
			char buf2[MAXLINE],buf3[MAXLINE];
			strcpy(buf2,buf);
			strcpy(buf3,buf);
			strtok(buf," ");
			nameTo = strtok(NULL," ");
			nameFrom = strtok(NULL," ");
			/*check if both users exist*/
			int userTo = 0, toFd , userFrom = 0, fromFd;
			User *temp = userHead;

			while(temp != NULL){
				if(!strcmp(nameTo,temp->name)){
					userTo = 1;
					toFd = temp->clientSock;
				}
				if(!strcmp(nameFrom,temp->name)){
					userFrom = 1;
					fromFd = temp->clientSock;
				}
				temp = temp->next;
			}

			if(userTo && userFrom){

				writeV(toFd,buf2,MAXLINE);
				
				writeV(fromFd,buf3,MAXLINE);
				

			}else{
				if(userTo)
					handleError(notAvailable,toFd);
				else
					handleError(notAvailable,fromFd);
				
			}

		}

	}

	return NULL;
}


void* addUser(char *name, void *pair){

	/*find last user*/
	User *temp = userHead;
	if(temp != NULL){
		while(temp->next!=NULL)
			temp = temp->next;
	}
	communicatePair p = *((communicatePair*)pair);
	User *newUser = malloc(sizeof(User));
	strcpy(newUser->name,name);
	newUser->clientSock = p.fd;
	newUser->addr = p.addr;
	newUser->next = NULL;
	newUser->loginTime = time(0);
	//printf("add user %s\n",newUser->name);
	if(userHead==NULL)
		userHead = newUser;
	else
		temp->next = newUser;

	return (void*)newUser;
}

void removeUser(int fd){

	User *temp, *prev;
	char buf[MAXLINE];
	char name[1000];
	temp = userHead;
	prev = temp;
	/*find user to remove*/
	while(temp->clientSock!=fd){
		prev = temp;
		temp = temp->next;
	}
	/*clean up */
	if(temp == userHead){
		if(temp->next != NULL){
			userHead = userHead->next;
		}else{
			userHead = NULL;
		}
	}
	

	strcpy(name,temp->name);

	Close(temp->clientSock);
	prev->next = temp->next;
	free(temp);

	/*send uoff to all other users*/
	strcpy(buf,"UOFF ");
	strcat(buf,name);
	strcat(buf," \r\n\r\n");
	temp = userHead;
	while(temp!=NULL){
		writeV(temp->clientSock,buf,MAXLINE);
		temp = temp->next;
	}
	
}

void handleError(int error_code,int fd){
	if(error_code==nameTaken){
		writeV(fd,"ERR 00 USER NAME TAKEN \r\n\r\nBYE \r\n\r\n",50);
		/*read bye from client*/
		char *temp = malloc(8);
		Read(fd,temp,8);
		if(!strcmp(temp,"BYE \r\n\r\n")){
			Close(fd);
		}
		free(temp);
	}else if(error_code==notAvailable){
		writeV(fd,"ERR 01 USER NOT AVAILABLE \r\n\r\nBYE \r\n\r\n",50);
		Close(fd);
	}else if(error_code==badPassword){
		writeV(fd,"ERR 02 BAD PASSWORD \r\n\r\nBYE \r\n\r\n",50);
		Close(fd);
	}else{

	}

}


int checkLogin(char *name, int exist){
	/*if user already login in, return 0*/
	int currentlyIn = 1, hasAccount = 0;
	User *temp = userHead;
	while(temp!=NULL){
		if(!strcmp(temp->name,name))
			currentlyIn = 0;
		temp = temp->next;
	}

	accountList *acc = accHead;
	while(acc!=NULL){
		if(!strcmp(acc->name,name)){
			hasAccount = 1;
			break;
		}
		acc = acc->next;
	}

	if(exist){
		if(hasAccount){
			if(currentlyIn)
				return 1;
			else 
				return -1;
		}else{
			return -2; 
		}
	}
		
	else 
		return !hasAccount;

}

void stdinCommand(){
	char buf[MAXLINE];
	if(scanf("%s",buf)<0){
		color("red",1);
		fprintf(stderr,"Error reading standard input\n");
		color("white",1);
	}

	if(!strcmp(buf,"/users")){
		//printf("calling user function\n");
		users();

	}else if(!strcmp(buf,"/help")){
		//printf("calling help function\n");
		HELP();

	}else if(!strcmp(buf,"/shutdown")){
		//printf("calling shutdown function\n");
		shutDown();

	}else if(!strcmp(buf,"/accts")){
		accts();
	}
}

void users(){
	User *temp = userHead;
	while(temp!=NULL){
		printf("User Name: %s FD: %d\n",temp->name,temp->clientSock);
		temp = temp->next;
	}
}

void accts(){
	accountList *temp = accHead;
	color("green",1);
	while(temp!=NULL){
		printf("User: %s\nPassword: %s\nSalt: %s\n",
			temp->name,temp->pwd,temp->salt);
		temp = temp->next;
	}
	color("white",1);
}

void shutDown(){
	cleanUp();
	exit(0);
}

void cleanUp(){

	User *temp = userHead;
	while(temp!=NULL){
		User *temp2 = temp;
		writeV(temp2->clientSock,"BYE \r\n\r\n",8);
		Close(temp->clientSock);
		temp = temp->next;
		free(temp2);
	}

	accountList *acc = accHead;
	accountList *accP = acc;

	while(acc!=NULL){
		accP = acc;
		acc = acc->next;
		free(accP);
	}

	Close(listenfd);
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
		color("red",1);
		fprintf(stderr,"Error on create thread\n");
		color("white",1);
	}
	/*wait for login thread to determine
	Pthread_join(tid,NULL);

	fprintf(stdout,"connected address is : %d \n",
		((struct sockaddr_in*)&clientaddr)->sin_addr.s_addr);
		*/


}

int getTime(time_t current_time){

	time_t newTime;
	time(&newTime);
	
	return (int)difftime(newTime,current_time);
}

void Select(int n,fd_set *set){
	if(select(n,set,NULL,NULL,NULL)<0){
		color("red",1);
		fprintf(stderr,"Error on select\n");
		color("white",1);
	}
}

int Accept(int socket, struct sockaddr *addr, socklen_t *socklen){
	int result = accept(socket, addr, socklen);
	if(result == -1){
		color("red",1);
		fprintf(stderr,"Error in accept connection: %s\n",strerror(errno));
		color("white",1);
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
	if(resultt != 0){
		color("red",1);
		fprintf(stderr,"Getaddrinfo with error: %s\n",gai_strerror(resultt));
		color("white",1);}
	return resultt;

}

void HELP(){
	fprintf(stdout,"Client Usage:\n \
	./server [-hv] SERVER_PORT MOTD\n \
	-h					Displays this help menu, and returns EXIT_SUCCESS.\n \
	-v					Verbose print all incoming and outgoing protocol verbs&content.\n \
	SERVER_PORT			Port number of server\n \
	MOTD				Message of today\n \
	/users				Print a list of user currently logged in\n \
	/accts 				Print a list of accounts \n \
	/help				Print this list of commands	and exit\n \
	/shutDown			Close all socket, files and free heap memory then terminate\n");
	shutDown();
	exit(0);
}

ssize_t Read(int fd, void *buf,size_t count){
	ssize_t result = read(fd,buf,count);
	if(result == -1){
		color("red",1);
		fprintf(stderr,"Error on read: %s\n",strerror(errno));
		color("white",1);
	}
	return result;
}

int Pthread_join(pthread_t tid, void **thread_return){
	int result = pthread_join(tid,thread_return);
	if(result!=0){
		color("red",1);
		fprintf(stderr,"Error join thread\n");
		color("white",1);
	}

	return result;
}

int Pthread_detach(pthread_t tid){
	int result = pthread_detach(tid);
	if(result!=0){
		color("red",1);
		fprintf(stderr,"Error detach thread\n");
		color("white",1);
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

int writeV(int fd, char *s, int byte){
	int result = write(fd,s,byte);
	if(verbose){
		color("green",1);
		printf("%s\n",s);
		color("white",1);
	}
	if(result == -1){
		color("red",1);
		fprintf(stderr,"Error on writing to FD: %d\n Error: %s\n",fd,strerror(errno));
		color("white",1);
	}
	return result;
}

int Close(int fd){
	int result;
	if((result = close(fd))== -1){
		color("red",1);
		fprintf(stderr,"Error on closing file, file descriptor: %d\n	\
		Error: %s\n",fd,strerror(errno));
		color("white",1);
	}
	return result;
}