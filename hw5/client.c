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

struct args{
	char arg[MAXLINE];
	struct args *next;
};
typedef struct args args;

args *head = NULL;
char cc = 0x1B;
char bb = 0x5B;
char username[20];
char host[20];
char port[20];
int clientfd;

void sigInt_handler(int sigID){
	write(clientfd,"BYE \r\n\r\n",8);
	char buf[MAXLINE];
	read(clientfd,buf,8);
	if(!strcmp(buf,"BYE \r\n\r\n")){
		close(clientfd);
		exit(EXIT_SUCCESS);
	}else{
		printf("Error handling signal INT");
	}
}

int main (int argc, char ** argv) {

	signal(SIGINT,sigInt_handler);

	int opt = 0;
	while((opt = getopt(argc,argv,"hcv")) != -1){
		if(opt == 'h'){
			HELP();
			exit(EXIT_SUCCESS);
		}else if(opt == 'c'){

		}
		else if(opt == 'v'){

		}

	}

	if(argc<4){
		errorPrint();
		fprintf(stderr,"Missing arguments \n");
		HELP();
		exit(0);
	}

	strcpy(username,argv[1]);
	strcpy(host,argv[2]);
	strcpy(port,argv[3]);

	if((clientfd=open_clientfd(argv[2],argv[3])) < 0){
		errorPrint();
		fprintf(stderr,"Open client fd failed\n");
		exit(0);

	}


	/*		

	

	if(login()<0){
		errorPrint();
		fprintf(stderr, "Login response failed\n" );
		Close(clientfd);
		exit(0);
	}
	*/
	fd_set read_set, ready_set;
	FD_ZERO(&read_set);
	FD_SET(STDIN_FILENO,&read_set);
	FD_SET(clientfd,&read_set);

	/*loop to see where input is from*/
	while(1){
		ready_set = read_set;
		Select(clientfd+1,&ready_set);
		/*check for input from stdin*/
		if(FD_ISSET(STDIN_FILENO,&ready_set)){
			stdinCommand();

		}


		if(FD_ISSET(clientfd,&ready_set))
			serverCommand(clientfd);

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


	write(clientfd,"WOLFIE \r\n\r\n",11);
	read(clientfd,&buffer,11);

	if(!strncmp(buffer,"EIFLOW \r\n\r\n",11)){

		memset(&buffer,0,sizeof(buffer));
		strcat(buffer,"IAM ");
		strcat(buffer,username);
		strcat(buffer," \r\n\r\n");

		write(clientfd,&buffer,sizeof(buffer));

		char arguments[10][1024];

		parseArg(clientfd,arguments);

		if(!strcmp(arguments[0],nameBuffer)){
			//login sucess, print MOTD <message>

			color("green",1);

			printf("%s\n",arguments[1]);

			color("white",1);

			return 1;
		}else{
			//error occurs
			
			if(!strncmp(arguments[1],"BYE \r\n\r\n",8)){
				write(clientfd,"BYE \r\n\r\n",8);
			}

			return -1;
		}

	}else{
		return -1;
	}

}

void parseArg(int fd,char arguments[10][1024]){
	/*
	args *temp = malloc(sizeof(args));
	head = temp;
	char buf[MAXLINE];

	read(fd,buf,MAXLINE);
	char *tempC = strtok(buf,"\r\n\r\n");
	strcpy(temp->arg,tempC);
	strcat(temp->arg,"\r\n\r\n");
	printf("%s\n",tempC);

	while((tempC = strtok(NULL,"\r\n\r\n"))!=NULL){

		printf("aaa\n");
		args *Next = malloc(sizeof(args));

		strcpy(Next->arg,tempC);
		strcat(Next->arg,"\r\n\r\n");

		temp->next = Next;
		temp = temp->next;
	}
	*/

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
		write(clientfd,"BYE \r\n\r\n",8);
	}else if(!strcmp(buf,"/listu\n")){
		write(clientfd,"LISTU \r\n\r\n",10);
	}else if(!strcmp(buf,"/time\n")){
		write(clientfd,"TIME \r\n\r\n",9);
	}else if(!strncmp(buf,"/chat",5)){
		startChatHandler(buf);
	}

}

void serverCommand(int clientfd){
	char buffer[50];
	memset(buffer,0,sizeof(buffer));
	read(clientfd,&buffer,sizeof(buffer));

	//handle shut down server command
	if(!strncmp(buffer,"BYE \r\n\r\n",8)){
		Close(clientfd);
		exit(EXIT_SUCCESS);
	}
	else if(!strncmp(buffer,"UTSIL",5)){
		listuHandler(buffer);
	}else if(!strncmp(buffer,"EMIT",4)){
		timeHandler(buffer);
	}else if(!strncmp(buffer,"MSG",3)){
		openChatHandler(buffer);
	}

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

	int i =0;
	int j =0;
	int index=0;

	while(i<strlen(buf)){

		if(index!=2&&buf[i]==' '){
			index++;
			j=0;
		}

		output[index][j]=buf[i];
		i++;
		j++;
	}

	//construct output protocol
	strcat(buffer,"MSG ");
	strcat(buffer,output[1]);
	strcat(buffer," ");
	strcat(buffer,username);
	strcat(buffer," ");
	strcat(buffer,output[2]);
	strcat(buffer," \r\n\r\n");

	write(clientfd,buffer,sizeof(buffer));

}
void openChatHandler(char*buf){
	  struct sockaddr_un addr;
	//  char buffer[100];
	  int socketFD;
	  char* socket_path = "./socket";
	  pid_t pid;
	  int status = 0;
	  char *arguments[11];
	  memset(arguments,0,sizeof(arguments));
	  arguments[0]="xterm";
	  arguments[1]="-hold";
	  arguments[2]="-geometry";
	  arguments[3]="45x40";
	  arguments[4]="-T";
	  arguments[5]="Chat Room";
	  arguments[6]="-e";
	  arguments[7]="./chat";
	 // arguments[8]="0000000000000";
	  arguments[9]=NULL;
	 
	  if ( (socketFD = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
	    perror("socket error");
	    exit(-1);
	  }

	  memset(&addr, 0, sizeof(addr));
	  addr.sun_family = AF_UNIX;
	  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

	  unlink(socket_path);

	  if (bind(socketFD, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
	    perror("bind error");
	    exit(-1);
	  }

	  pid = fork();

	  if(pid==0){
	  		printf("in child socket:%d \n",socketFD );
	  		char temp[10];
	  		memset(temp,0,10);
	  		sprintf(temp,"%d",socketFD);
	  		arguments[8]= temp;
	  		
	  		execvp(arguments[0],arguments);
	  	//	exit(EXIT_SUCCESS);
	  }else{
	  	if(waitpid(pid,&status,0) >= 0){


	  	}else{

		fprintf(stderr,"Child terminated abnormally with error:%s\n",strerror(errno));

	  	}

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
