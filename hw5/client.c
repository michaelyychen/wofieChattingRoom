#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
#include <termios.h>

#include "myHeader.h"

#define MAXLINE 1024

struct args{
	char arg[MAXLINE];
	struct args *next;
};
typedef struct args args;

struct childList{
	char user[50];
	int fd;
	struct childList* next; 	
};typedef struct childList childList;

args *head = NULL;
char cc = 0x1B;
char bb = 0x5B;
char username[20];
char host[20];
char port[20];
int verbose =0;
int clientfd;
int newuser = 0;

fd_set read_set,ready_set;
struct childList* childHead=NULL;

void addChild(int fd,char* name){
	childList *child = malloc(sizeof(childList));
	strcpy(child->user,name);
	child->fd=fd;
	//child->user name;
	child->next = NULL;
	childList *temp = childHead;
	if(temp!=NULL){
		while(temp->next!=NULL){
			temp = temp->next;
		}
	
	temp->next = child;
	}else{

		childHead = child;
	}

}
void sigInt_handler(int sigID){
	writeV(clientfd,"BYE \r\n\r\n",8);
	char buf[MAXLINE];
	read(clientfd,buf,8);
	if(!strcmp(buf,"BYE \r\n\r\n")){
		shutDown();
	}else{
		printf("Error handling signal INT");
	}
}

void sigChild_handler(int sigID){

	if (fcntl(childHead->fd, F_GETFL) < 0 && errno == EBADF) {
    // file descriptor is invalid or closed
		printf("i got it\n");
	}

}
int main (int argc, char ** argv) {

	signal(SIGINT,sigInt_handler);
	/*signal(SIGCHLD,sigChild_handler);*/


	if(argc<4){
	errorPrint();
	fprintf(stderr,"Missing arguments \n");
	HELP();
	exit(0);
	}

	strcpy(username,argv[1]);

	strcpy(host,argv[2]);
	strcpy(port,argv[3]);

	int opt = 0;
	while((opt = getopt(argc,argv,"hcv")) != -1){
		if(opt == 'h'){
			HELP();
			exit(EXIT_SUCCESS);
		}else if(opt == 'c'){
			newuser = 1;
		}
		else if(opt == 'v'){
			verbose=1;
		}

	}

	if((clientfd=open_clientfd(host,port)) < 0){
		errorPrint();
		fprintf(stderr,"Open client fd failed\n");
		exit(0);

	}

	

	
	if(login()<0){
		errorPrint();
		fprintf(stderr, "Login response failed\n" );
		Close(clientfd);
		exit(0);
	}

	FD_ZERO(&read_set);
	FD_SET(STDIN_FILENO,&read_set);
	FD_SET(clientfd,&read_set);
	//childList *temp = childHead;
	/*loop to see where input is from*/
	while(1){
		ready_set=read_set;
		int wait = findlast();

		if(wait ==0){
			wait=clientfd;
			}		
		Select(wait+1,&ready_set);

		
		/*check for input from stdin*/
		if(FD_ISSET(STDIN_FILENO,&ready_set))
			stdinCommand();
		
		if(FD_ISSET(clientfd,&ready_set))
			serverCommand(clientfd);


		childList *temp = childHead;
		while(temp!=NULL){
			
			if(FD_ISSET(temp->fd,&ready_set)){
				childCommand(temp->fd);
			}
			if(temp->next==NULL){
				break;
			}
			temp = temp->next;
		}
		
	}

	Close(clientfd);
	exit(0);
}

//positive = login success o.w. failed
int login(){
	char buffer[MAXLINE];
	char nameBuffer[20];		//HI_<name>_\r\n\r\n

	memset(nameBuffer,0,sizeof(nameBuffer));
	strcat(nameBuffer,"HI ");
	strcat(nameBuffer,username);
	strcat(nameBuffer," \r\n\r\n");


	writeV(clientfd,"WOLFIE \r\n\r\n",11);
	read(clientfd,&buffer,11);

	if(!strncmp(buffer,"EIFLOW \r\n\r\n",11)){

		memset(&buffer,0,sizeof(buffer));
		/*if user wants to create new acct*/
		if(newuser){
			/*tell server iam new*/
			strcpy(buffer,"IAMNEW ");
			strcat(buffer,username);
			strcat(buffer," \r\n\r\n");

			writeV(clientfd,buffer,12+strlen(username));
			printf("%lu\n",12+strlen(username));
			char arguments[10][1024];

			parseArg(clientfd,arguments);

			strcpy(buffer,"HINEW ");
			strcat(buffer,username);
			strcat(buffer," \r\n\r\n");

			if(!strcmp(arguments[0],buffer)){
				/*prompt user for password*/
				char p[64];
				memset(p,0,64);
				promtPwd(p);
				memset(buffer,0,MAXLINE);
				strcpy(buffer,"NEWPASS ");
				strcat(buffer,p);
				strcat(buffer," \r\n\r\n");
				writeV(clientfd,buffer,13+strlen(p));
				printf("%lu\n",13+strlen(p));
				/*read response from server*/
				char arguments[10][1024];

				parseArg(clientfd,arguments);

				if(!strcmp(arguments[1],nameBuffer)){

					color("green",1);

					printf("%s\n",arguments[2]);

					color("white",1);

					return 1;
				}else{

					printf("Error bad password\n");
					return -1;
				}


			}else{
				printf("Error bad username\n");
				return -1;
			}

		}else{
			/*login process for exist user*/
			strcat(buffer,"IAM ");
			strcat(buffer,username);
			strcat(buffer," \r\n\r\n");

			writeV(clientfd,buffer,9+strlen(username));
			printf("%lu\n",9+strlen(username));
			memset(buffer,0,MAXLINE);
			read(clientfd,buffer,MAXLINE);

			/*check if is auth*/
			char buf[MAXLINE];
			strcpy(buf,"AUTH ");
			strcat(buf,username);
			strcat(buf," \r\n\r\n");

			if(!strcmp(buf,buffer)){
				char pwd[64];
				memset(pwd,0,64);
				promtPwd(pwd);
				
				/*send pwd to server*/
				memset(buffer,0,MAXLINE);
				strcpy(buffer,"PASS ");
				strcat(buffer,pwd);
				strcat(buffer," \r\n\r\n");
				writeV(clientfd,buffer,10+strlen(pwd));
				printf("%lu\n",10+strlen(pwd));
				char arguments[10][1024];
				parseArg(clientfd,arguments);

				if(!strcmp(arguments[0],"SSAP \r\n\r\n")){
					if(!strcmp(arguments[1],nameBuffer)){
						char *token = strtok(arguments[2]," ");
						if(!strcmp(token,"MOTD")){
							token = strtok(NULL," ");
							color("green",1);
							printf("%s\n",token);
							color("white",1);	
						}else{
							fprintf(stderr,"Server did not pass back MOTD\n");
						}
					}else{
						fprintf(stderr,"Server did not pass back HI\n");
						return -1;
					}


				}else{
					fprintf(stderr,"Bad Password\n");
					return -1;
				}
			}else{
				fprintf(stderr,"User name exist\n");
				return -1;
			}
		}

	}else{
		return -1;
	}

	return 1;
}

int promtPwd(char *pwd){

	struct termios oflags, nflags;
 
    /* disabling echo */
    tcgetattr(fileno(stdin), &oflags);
    nflags = oflags;
    nflags.c_lflag &= ~ECHO;
    nflags.c_lflag |= ECHONL;

    if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
        perror("tcsetattr");
        return -1;
    }

    printf("password: ");
   
    fgets(pwd,64,stdin);
    pwd[strlen(pwd) - 1] = 0;
  

    /* restore terminal */
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
        perror("tcsetattr");
        return -1;
    }

    return 1;
}

void parseArg(int fd,char arguments[10][1024]){

	char buf[MAXLINE],*temp;
	read(fd,buf,MAXLINE);
	//printf("%s\n",buf);
	char con[4] = "\r\n\r\n";
	temp = strtok(buf,con);
	int i = 0;
	strcpy(arguments[i],temp);
	strcat(arguments[i],con);
		//printf("%s\n",temp);
	while((temp=strtok(NULL,con))!=NULL){
		i++;

		strcpy(arguments[i],temp);

		strcat(arguments[i],con);
	}

}

void stdinCommand(){

	char buf[MAXLINE];
	if(!fgets(buf,MAXLINE,stdin))
		exit(0);
	if(!strcmp(buf,"/help\n")){
		helpCommand();
	}else if(!strcmp(buf,"/logout\n")){
		writeV(clientfd,"BYE \r\n\r\n",8);
	}else if(!strcmp(buf,"/listu\n")){
		writeV(clientfd,"LISTU \r\n\r\n",10);
	}else if(!strcmp(buf,"/time\n")){
		writeV(clientfd,"TIME \r\n\r\n",9);
	}else if(!strncmp(buf,"/chat",5)){
		startChatHandler(buf);
	}

}

void serverCommand(int clientfd){
	char buffer[MAXLINE];
	memset(buffer,0,sizeof(buffer));
	read(clientfd,&buffer,sizeof(buffer));

	//handle shut down server command
	if(!strncmp(buffer,"BYE \r\n\r\n",8)){
		shutDown();
	}
	else if(!strncmp(buffer,"UTSIL",5)){
		listuHandler(buffer);
	}else if(!strncmp(buffer,"EMIT",4)){
		timeHandler(buffer);
	}else if(!strncmp(buffer,"MSG",3)){
		openChatHandler(buffer);
	}
	else if(!strncmp(buffer,"UOFF",4)){
		uoffHandler(buffer);
	}
	else if(!strncmp(buffer,"ERR",3)){
		errorHandler(buffer);
	}


}

void uoffHandler(char* buffer){


	int fd =0;
	childList* ptr = childHead;
/*
	while(buffer[index]!=' '){
		temp[i]=buffer[index];
		index++;
		i++;
	}*/
	char *token = strtok(buffer," ");
	token = strtok(NULL," ");
	
	while(ptr!=NULL){
		if(!strcmp(ptr->user,token)){
			fd = ptr->fd;
			break;
		}
		ptr = ptr->next;
	}

	if(fd!=0){
		printf("remove %s\n",token );
		write(fd,"disconnect",10);
	}

}

void errorHandler(char* buffer){
	char temp[100];
	int index =4;
	int i =0;
	

	while(buffer[index]!=' '){
		temp[i]=buffer[index];
		index++;
		i++;
	}
	errorPrint();
	if(!strncmp(temp,"00",2)){
		
		fprintf(stderr, "USER NAME TAKEN\n");
	}else if(!strncmp(temp,"01",2)){
		fprintf(stderr, "USER NOT AVALIABLE\n");
		return;
	}else if(!strncmp(temp,"02",2)){
		fprintf(stderr, "BAD Password\n");
	}else{
		fprintf(stderr, "INTERNAL SERVER ERROR\n");
	}

	shutDown();
}
void removeChild(char* buffer){
	char fdToRemove[50];
	int index = 5;
	int i =0;
	while(buffer[index]!=' '){
		fdToRemove[i]=buffer[index];
		i++;
		index++;
	}

	int fd = stringToInt(fdToRemove);
	
	childList * tempP = childHead;
	childList * temp = childHead;

	/*find user to remove*/
	while(temp->fd==fd){
		tempP = temp;
		temp = temp->next;
	}
	/*clean up */
	if(temp == childHead){
		childHead=NULL;
	}

	Close(temp->fd);
	tempP->next = temp->next;
	free(temp);


	
}
void childCommand(int fd){

	char buffer[1024];
	char msgTo[50];
	memset(buffer,0,sizeof(buffer));
	//readin child input
	read(fd,&buffer,sizeof(buffer));


	if(!strncmp(buffer,"remove",6)){
		removeChild(buffer);
		return;
	}

	char responseBUf[1024];

	//close(pair[childsocket]);

	
	//writeV(pair[parent],temp,sizeof(temp));
	
	childList *temp = childHead;
    while(temp!=NULL){
    	if(temp->fd==fd){
    		strcpy(msgTo,temp->user);
    	}
    	if(temp->next==NULL){
    		break;
    	}
    	temp = temp->next;
    }

    memset(responseBUf,0,1024);

    strcat(responseBUf,"MSG ");
    strcat(responseBUf,msgTo);
	strcat(responseBUf," ");
	strcat(responseBUf,username);
	strcat(responseBUf," ");
 	strcat(responseBUf,buffer);
	strcat(responseBUf," \r\n\r\n");
	writeV(clientfd,responseBUf,11+strlen(msgTo)+strlen(username)+strlen(buffer));
	printf("%lu\n", 11+strlen(msgTo)+strlen(username)+strlen(buffer));
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

void timeHandler(char* buf){
	char buffer[50];
	char temp[3][50];
	char* token;
	int index =0;
	int hour,minute,second;
	strcpy(buffer,buf);

	token = strtok(buffer," ");

	while(token!=NULL){
		strcpy(temp[index],token);
		token = strtok(NULL," ");
		index++;
	}

	int timeInSec=stringToInt(temp[1]);

	hour=timeInSec/3600;
	minute=(timeInSec%3600)/60;
	second=timeInSec%60;

	color("blue",1);
	fprintf(stdout, "Connected for ");
	color("white",1);
	fprintf(stdout, "%d hour(s) %d minute(s) and %d seconds(s)\n",
						hour,minute,second );
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

void listuHandler(char* buffer){
	char temp[50][50];
	char* token;
	int index =0;
	int i =1;
	token = strtok(buffer," ");

	while(token!=NULL){
		strcpy(temp[index],token);
		token = strtok(NULL," ");
		index++;
	}


	while(i<(index-1)){


		fprintf(stdout, "user: %s\n",temp[i]);
		i=i+2;
	}

}
void startChatHandler(char*buf){

	char buffer[MAXLINE];
	//format the buffer so that [0]=/chat [1]=<TO> [2]=message
	char output[3][MAXLINE];

	memset(buffer,0,1024);
	

	char *token = strtok(buf," ");
	token = strtok(NULL," ");
	strcpy(output[1],token);
	token = strtok(NULL," ");
	strcpy(output[2],token);
	
	printf("%s\n",output[1] );	
	//construct output protocol
	strcat(buffer,"MSG");
	strcat(buffer," ");
	strcat(buffer,output[1]);
	strcat(buffer," ");
	strcat(buffer,username);
	strcat(buffer," ");
	strcat(buffer,output[2]);
	strcat(buffer," \r\n\r\n");

	writeV(clientfd,buffer,11+strlen(output[1])+strlen(output[2])+strlen(username));
	printf("%lu\n",11+strlen(output[1])+strlen(output[2])+strlen(username));
}

void openChatHandler(char*buf){

	  pid_t pid;
	  
	  char msgTo [20];
	  char msgFrom [20];
	  char msg[MAXLINE];

	  char *arguments[11];
	  memset(arguments,0,sizeof(arguments));
	  arguments[0]="xterm";
	  //arguments[1]="-hold";
	  arguments[1]="-geometry";
	  arguments[2]="45x40";
	  arguments[3]="-T";
	  //arguments[4]="Chat Room: ";
	  arguments[5]="-e";
	  arguments[6]="./chat";
	  arguments[9]=NULL;

	  char title[] = "Chat Room: ";
	  char output[1024];
 	  int window=0;
	  int pair[2];
	  //arrow true = > incoming msg else '<' outgoing
	  
	  parseMSG(buf,msgTo,msgFrom,msg);


	   memset(output,0,sizeof(output));
	  //if from=myself, check if we have chat window open with msgTo 
	  if(!strcmp(msgFrom,username)){
	  	window = windowCheck(msgTo);
	  	strcat(title,msgTo);
	  	arguments[4]=title;
	  	strcat(output,"<");
	  }
	  else{
	  	window= windowCheck(msgFrom);
	  	strcat(title,msgFrom);
	  	arguments[4]=title;
	  	strcat(output,">");
	  }
	  //don't need to fork, writeV message to child directly
	  if(window>0){

	  	strcat(output,msg);
	  	writeV(window,output,sizeof(output));


	  }else{

		  static const int parent = 0;
		  static const int child = 1;

		  if(socketpair(AF_UNIX,SOCK_STREAM,0,pair)<0){
		  	fprintf(stderr, "socketpair Error\n" );
		  }
	  
		  if((pid = fork())==0){

		  		char temp[10];
		  		memset(temp,0,10);
		  		sprintf(temp,"%d",pair[child]);
		  		arguments[7]= temp;

		  		close(pair[parent]);
			
		  		execvp(arguments[0],arguments);
		  		

		  }else{
		  		strcat(output,msg);
		  		writeV(pair[parent],output,strlen(output));
		  		printf("%lu\n",strlen(output) );
		  		close(pair[child]);
		  		FD_SET(pair[parent],&read_set);

	  			if(!strcmp(msgFrom,username)){
				  	addChild(pair[parent],msgTo);
				}
				  else{	  	
		  			addChild(pair[parent],msgFrom);
				}


		  }
	


	  }
	 

}
int windowCheck(char*user){
	//user can't be username
	childList *temp = childHead;
	if(temp==NULL){
			return -1;
	}else{
		while(temp!=NULL){
			if(!strcmp(temp->user,user)){
				return temp->fd;
			}
			if(temp->next==NULL){
				break;
			}
			temp= temp->next;
		}
		return -1;
	}


}
void parseMSG(char*buf,char*msgTo,char*msgFrom,char*msg){
	  int index = 4;
	  int i =0;

	  memset(msgTo,0,20);
	  memset(msgFrom,0,20);
	  memset(msg,0,1024);
	  //parse msgTo 
	  while(buf[index]!=' '){
	  	msgTo[i]=buf[index];
	  	i++;
	  	index ++;
	  }
	  index ++;
	  i =0;
	  //parse msgFrom
	  while(buf[index]!=' '){
	  	msgFrom[i]=buf[index];
	  	i++;
	  	index ++;
	  }
	  index ++;
	  i =0;
	  while(buf[index]!='\r'){
	  	msg[i]=buf[index];
	  	i++;
	  	index ++;
	  }

}
void Select(int n,fd_set *set){
	if(select(n,set,NULL,NULL,NULL)<0)
		fprintf(stderr,"Error on select\n");
}

void HELP(){
	color("yellow",1);
	fprintf(stdout,"Client Usage:\n" );
	color("white",1);
	fprintf(stdout,
	"./client [-hcv] <NAME> <SERVER_IP> <SERVER_PORT>							\n \
	-h		Displays this help menu, and returns EXIT_SUCCESS.	\n \
	-c		Requests to server to create a new user				\n \
	-v		Verbose print all incoming and outgoing protocol verbs&content.			\n \
	NAME		This is the username to display when chatting.	\n \
	SERVER_IP	The IP Address of the server to connect to.		\n \
	SERVER_PORT	The port to connect to.	\n");
}

void helpCommand(){
	color("yellow",1);
	fprintf(stdout,"Client Commands:\n" );
	color("white",1);
	fprintf(stdout,
	"/time		Show how long have been connected to the server.\n \
	/logout		Log out from the server.				\n \
	/help		List all the commands accepted by the program.			\n \
	/listu		List all the user currently on the server.	\n \
	/chat		Starting a chat with someone.	\n ");
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

void errorPrint(){
	color("red",2);
	fprintf(stderr, "error: " );
	color("white",2);

}

int findlast(){
	childList* temp = childHead;

	int max =0;
	if(temp==NULL){
		return 0;
	}else{
		while(temp!=NULL){
			if(max<(temp->fd)){	
				max = temp->fd;
			}
			if(temp->next==NULL){
				break;
			}
			temp = temp->next;
		}

		return max;
	}

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


void shutDown(){
	childList* temp = childHead;
	childList* tempP = temp;

	while(temp!=NULL){
		tempP=temp;
		write(tempP->fd,"disconnect",10);
		Close(tempP->fd);
		temp=temp->next;
		free(tempP);
		
	}

	Close(clientfd);
	exit(EXIT_SUCCESS);


}