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

/*ready to login*/
	if(login()<0){
		errorPrint();
		fprintf(stderr, "Login response failed\n" );
		Close(clientfd);
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

//positive = login success o.w. failed
int login(){
	char buffer[50];
	char nameBuffer[20];		//HI_<name>_\r\n\r\n

	strcat(nameBuffer,"HI ");
	strcat(nameBuffer,username);
	strcat(nameBuffer," \r\n\r\n");


	write(clientfd,"WOLFIE \r\n\r\n",11);
	read(clientfd,&buffer,11);

	if(!strncmp(buffer,"EIFLOW \r\n\r\n",11)){
		write(clientfd,"IAM ",4);
		write(clientfd,&username,sizeof(username));
		write(clientfd," \r\n\r\n",5);

		read(clientfd,&buffer,sizeof(buffer));


		if(!strcmp(buffer,nameBuffer)){
			//login sucess, print MOTD <message>
			read(clientfd,&buffer,sizeof(buffer));
			color("green");
			fprintf(stdout, "%s\n",buffer );	
			color("white");

			return 1;
		}else{
			//error occurs
			read(clientfd,&buffer,sizeof(buffer));
			if(!strncmp(buffer,"BYE \r\n\r\n",8)){
				write(clientfd,"BYE \r\n\r\n",8);
			}

			return -1;	
		}

	}else{
		return -1;	
	}

}
void stdinCommand(){
	char buf[MAXLINE];
	if(!fgets(buf,MAXLINE,stdin))
		exit(0);
	if(!strcmp(buf,"/help\n")){
		helpCommand();
	}else if(!strcmp(buf,"/logout\n")){
		logoutHandler();
	}else if(!strcmp(buf,"/listu\n")){
		listuHandler();
	}else if(!strcmp(buf,"/time\n")){
		timeHandler();
	}else if(!strncmp(buf,"/chat",5)){
	
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
	color("yellow");
	fprintf(stdout,"Client Usage:\n" );
	color("white");
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
	color("yellow");
	fprintf(stdout,"Client Commands:\n" );
	color("white");
	fprintf(stdout,
	"	/time		Show how long have been connected to the server.\n \
	/logout		Log out from the server.				\n \
	/help		List all the commands accepted by the program.			\n \
	/listu		List all the user currently on the server.	\n \
	/chat		Starting a chat with someone.	\n ");
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

void listuHandler(){
	char buffer[1024];

	write(clientfd,"LISTU \r\n\r\n",10);
	read(clientfd,&buffer,sizeof(buffer));
	if(!strncmp(buffer,"UTSIL \r\n\r\n",10)){
		int i;
		for(i=6;i<strlen(buffer);i++){
			fprintf(stdout, "%c",buffer[i]);
		}
	}
}

void logoutHandler(){
	char buffer[50];

	write(clientfd,"BYE \r\n\r\n",8);
	read(clientfd,&buffer,sizeof(buffer));
	if(!strncmp(buffer,"BYE \r\n\r\n",8)){
		Close(clientfd);
		exit(EXIT_SUCCESS);
	}

}
void timeHandler(){
	char buffer[50];
	char temp[3][50];
	char* token;
	int index =0;
	int hour,minute,second;

	write(clientfd,"TIME \r\n\r\n",9);
	read(clientfd,&buffer,sizeof(buffer));

	token = strtok(buffer," ");

	while(token!=NULL){
		strcpy(temp[index],token);
		token = strtok(NULL," ");
		index++;
	}

	int timeInSec=stringToInt(temp[1]);

	hour=timeInSec/3600;
	minute=timeInSec/60;
	second=timeInSec%60;

	color("blue");
	fprintf(stdout, "Connected for ");
	color("white");
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