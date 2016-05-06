#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/select.h>
#include <termios.h>
#include <semaphore.h>
#include "myHeader.h"
#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

	char buffer[1024];
	char path[50];
	char * ptr;
	FILE *logS;
	int logFD;
	char *arguments[11];
	int length, i = 0;
	int fd;
	int wd;
	char buf[EVENT_BUF_LEN];
	fd_set read_set,ready_set;

int main (int argc, char ** argv) {

	  if(argc<2){

	  fprintf(stderr,"  Usage:\n" );
	  fprintf(stderr,
	  "	./tool LOGFILE \n 	LOGFILE The Log file to be open\n");
	  exit(0);

	  }
	strcpy(path,argv[1]);
	createLog();


	fd = inotify_init();
	  /*checking for error*/
	if ( fd < 0 ) {
	    perror( "inotify_init" );
	}

  
 	wd = inotify_add_watch( fd, path, IN_MODIFY);

	FD_ZERO(&read_set);
	FD_SET(STDIN_FILENO,&read_set);
	FD_SET(fd,&read_set);

	while(1){
	//fprintf(stdout,"Enter Command: ");	

  	colors("green");
	fprintf(stdout,"Enter Command: ");
	fflush(stdout);
	colors("white");

	ready_set=read_set;

    select(fd+1,&ready_set,NULL,NULL,NULL);
	

	if(FD_ISSET(STDIN_FILENO,&ready_set))
		stdinCommand();
	
	if(FD_ISSET(fd,&ready_set))
		fileModified();

	}
	exit(0);

}


void createLog(){

	logFD = open(path,O_RDWR|O_CREAT,S_IWUSR|S_IRUSR|S_IXUSR);
		
	logS = fdopen(logFD,"r");
}

void run(){
	pid_t pid;
	if((pid = fork())==0){
			execvp(arguments[0],arguments);
	}else{
		if(waitpid(-1,NULL,0) >= 0){

		}
	}
}

void stdinCommand(){

	fgets(buffer,1024,stdin);
	char * index;


	if((index=strchr(buffer,'\n'))!=NULL)
        *index = '\0';

	if(!strcmp(buffer,"/close")){
		close(logFD);
		close(fd);
		inotify_rm_watch( fd, wd );
	    
        exit(EXIT_SUCCESS);
    }else if(!strncmp(buffer,"/help",5)){
    	helpCommand();
	
    }else if(!strncmp(buffer,"/search",7)){

    	ptr=strtok(buffer," ");	//search

    	ptr=strtok(NULL," ");	//serach criteria
    	if(ptr==NULL){
    		fprintf(stderr, "Missing Arguments\n" );
    		helpCommand();
    		return;
    	}
    	
    	arguments[0]="grep";
    	arguments[1]=ptr;
    	arguments[2]=path;
    	run();


    }else if(!strcmp(buffer,"/log")){
    	auditHandler();
    }else if(!strncmp(buffer,"/filter",7)){
    	ptr=strtok(buffer," ");// filter
    	ptr=strtok(NULL," "); //date or time

    	char start[50];
    	char end[50];
    	char temp [1024];
    	if(!strcmp(ptr,"date")){
    		ptr=strtok(NULL," ");
    		strcpy(start,ptr);
    		ptr=strtok(NULL," ");
    		strcpy(end,ptr);

	    	arguments[0]="sed";
	    	arguments[1]="-n";
	    	sprintf(temp,"/%s/,/%s/p",start,end);
	    	
	    	arguments[2]=temp;
	    	arguments[3]=path;

	    	run();
    	}else if(!strcmp(ptr,"time")){



    	}else{



    	}

    }else if(!strncmp(buffer,"/sort",5)){

    	ptr=strtok(buffer," ");	//sort
    	ptr=strtok(NULL," ");	//sort criteria

    	if(ptr==NULL){
    		fprintf(stderr, "Missing Arguments\n" );
    		helpCommand();
    		return;
    	}

    	if(!strcmp(ptr,"date")){
    		arguments[2]="1";
    	}else if(!strcmp(ptr,"time")){
    		arguments[2]="3";
    	}else if(!strcmp(ptr,"user")){
    		arguments[2]="4";
    	}else if(!strcmp(ptr,"event")){
    		arguments[2]="5";
    	}
    	ptr=strtok(NULL," ");	//ascending/descending
    	if(ptr==NULL){
    		fprintf(stderr, "Missing Arguments\n" );
    		helpCommand();
    		return;
    	}

    	if(!strncmp(ptr,"asc",3)){
	    	arguments[0]="sort";
	    	arguments[1]="-k";
	    	arguments[3]=path;
	    	arguments[4]=NULL;
    	}else if (!strncmp(ptr,"des",3)){

    		arguments[0]="sort";
	    	arguments[1]="-r";	
    		arguments[3]=arguments[2];
    		arguments[2]="-k";
    		arguments[4]=path;
    	}
    	run();

    }
}
void fileModified(){

  length = read( fd, buf, EVENT_BUF_LEN ); 

  /*checking for error*/
  if ( length < 0 ) {
    perror( "read" );
  }  
  colors("yellow");
  fprintf(stdout,"Entry has been Added: \n");
   colors("white");
  fflush(stdout);
 	arguments[0]="tail";
	arguments[1]="-n";	
	arguments[2]="1";
	arguments[3]=path;
	run();
}

void auditHandler(){

	int c;

   	fseek(logS,0,SEEK_SET);   


	while(1){
      c = fgetc(logS);
      if(feof(logS)){ 
         break ;
      }
      fprintf(stdout,"%c",c);
   	}

}

void colors(char* color){

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
void helpCommand(){
	colors("blue");
	fprintf(stdout,"Tool Commands:\n");
	fflush(stdout);
	colors("white");
	fprintf(stdout,"	/close			Exit the program.\n\
	/log 			Print file to stdout\n\
	/search <KEY>		Search	for the keyword			\n\
	/help			List all the commands accepted by the program.			\n\
	/sort <Column> <Order> Sort the specify column by ascending or descending.\n\
	/filter	<date/time> <start> <end> Filter based on any field.	\n");
}