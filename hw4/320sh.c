#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include "myHeader.h"
// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024
#define MAX_ARG 128

char *tokens[100];
char lastLocation[100];
char cmdHistory[50][MAX_INPUT];
int status = 0;
char historyLocation[100] = "";
char cc = 0x1B;
char bb = 0x5B;

int debug = 0;
char **ENVP;
char cmd[MAX_INPUT];
char direct = 0;
int IN = 0;
int OUT = 1;
int COUNT = 0;
int fd = 0;
int fpp;
int main (int argc, char ** argv, char **envp) {
  
  ENVP = envp;
  int finished = 0;
  char *prompt = "  320sh> ";
  
  int index = 0;
  /*debug flag*/
	int opt = 0;
	while((opt = getopt(argc,argv,"d")) != -1){
		if(opt == 'd')
			debug = 1;
	}

  splitPath(tokens);

  
  strcat(historyLocation ,getenv("HOME"));
  strcat(historyLocation,"/history.txt");
  fpp = OPEN(historyLocation,O_RDWR|O_CREAT,S_IWUSR|S_IRUSR|S_IXUSR);

  /*load cmdhistsory file*/
  historyFile(0);

  while (!finished) {
    char *cursor;
   // char *originalPos;
    char last_char;
    int rv;
    int count;
    int position = 0;
    index = 0;
    int historyCounter=0;

    //save leftmost cursor position
    saveCursor();
    // Print the prompt
   	printPromptDirectory();
    
    rv = write(1, prompt, strlen(prompt));
    if (!rv) { 
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, 
	  cursor = cmd, last_char = 1;
	  rv 
	  && (++count < (MAX_INPUT-1))
	  && (last_char != '\n');
	cursor++) { 
     // originalPos=cursor;

      rv = read(0, cursor, 1);
  	  
      last_char = *cursor;
     
      if(last_char == 0x1b){
      	cursor++;
      //	cmd[index]=0x1b;
      //	index ++;
      	read(0,cursor,1);

      	if(*cursor == 0x5b){
      	//	cmd[index]=0x5b;
      	//	index ++;
      		cursor++;
      		read(0,cursor,1);
		    
		    //up
		    if(*cursor == 0x41){
				
		    	if(strcmp(cmdHistory[historyCounter+1],"")!=0){

		    	 clearWholeLine();
		         //clean cmd
		         memset(cmd,0,1024); 
		         
		         //copy history to cmd
		         
		         strcpy(cmd,cmdHistory[historyCounter+1]);
		         historyCounter++;
		         index = strlen(cmd);
		         restoreCursor();
		         printPromptDirectory();
  				 write(1, prompt, strlen(prompt));
		         write(1,cmd,index);

		         position = index;
				        
		    	}
		          
		    //down
      		}
      		else if(*cursor == 0x42){

      			if(historyCounter>0){

		    	 clearWholeLine();
		         //clean cmd
		         memset(cmd,0,1024); 
		         
		         //copy history to cmd
		         
		         strcpy(cmd,cmdHistory[historyCounter-1]);
		         historyCounter--;
		         index = strlen(cmd);
		         restoreCursor();
		         printPromptDirectory();
  				 write(1, prompt, strlen(prompt));
		         write(1,cmd,index);

		         position = index;
				        
		    	}	
      		
      		}
      		else if(*cursor == 0x43){

      		if(position!=index){
          		 Right();
	      		position ++;
	      	}

      		}
      		else{

	      		if(position!=0){
	      	  	Left();
	      		position --;
	      		}
      
      		}

      	}
        //backspace
      }else if(last_char == 127){

          if(position>0){


          //save cursor location
          saveCursor();

          int temp = position;

          while(position!=0){
            Left();
            position --;
          }

          //clear line to the end
		  clearLine();

          //shift everything left by 1 unit
          position = temp;

          while(position<index){
            cmd[position-1]=cmd[position];
            position++;
          }

          cmd[index]='\0';
          index--;
          write(1,cmd,index);
          //restore cursor location
          restoreCursor();

          Left();
          position=temp-1; 

          }
         
      }   		  	
      //new line in case cursor is at the middle of input
      else if(last_char == 10){
      	
        while(position!=index){
          Right();
          position++;
        }
        write(1, &last_char, 1);
        cmd[index]=last_char;

      }	

      else if(last_char == 3) {
        write(1, "^c", 2);
     
      } else {

        //writes to the end of the string
        if(position == index){
        write(1, &last_char, 1);
        cmd[index]=last_char;
        


        index ++;
        position++;
        
        }else{
        //insert inplace

        int temp = position;
        //move everything to the right by 1 unit
        while(temp<=index){
          cmd[temp+1] = cmd[temp];
          temp++;
        }
        //insert the char
        cmd[position]=last_char;

        

        //save cursor location
        saveCursor();

        temp = position;

          while(position!=0){
            Left();
            position --;
          }
        clearLine(); 

        position = temp;

        index ++;
        position++;

        write(1,cmd,index);
        //restore cursor location
		restoreCursor();
        Right();

        }

      }

    } 

    *cursor = '\0';


    if (!rv) { 
      finished = 1;
      break;
    }

	
    // Execute the command, handling built-in commands separately 
    // Just echo the command line for now
    // write(1, cmd, strnlen(cmd, MAX_INPUT));
    saveHistory(cmd);
   
	eva(cmd);
    memset(&cmd,0,1024);

  }
 


  return 0;
}

void eva(char* cmd){

		char *argv[MAX_ARG]; /*Argument list*/
		char buf[MAX_INPUT]; /*Copy of command line*/
		int job;			 /*hold job type, background if 0, foreground otherwise*/
		
	
		
		strcpy(buf, cmd);

		/*parse command line*/
		job = parse(buf,argv);

		printf("job = %d\n",job);
		  
		/*check if command if empty*/
		if(argv[0] == NULL){
			return;
		}
		
		int build_in = 0;
		/*check to see if command is buildin, handle them if yes */
		buildIn(argv,&build_in);
	
			if((build_in == 0) && (checkRedir(argv) == 0)){
				/* if not a build, nor a redirection*/
				exe(argv);		
			}
		
}

void exe(char **argv){
	pid_t pid;			 /*new process id*/
		/*not build, fork a child process*/
				if((pid = fork()) == 0){
				
				/*if there is redirection need, direct file reference*/
				if(direct)
					directFile();
					
				/*find path of binary file*/
				char newPath[1028] = "";			
				findPath(argv[0],newPath);
				//printf("path is %s\n",newPath );
					
					if(newPath[0] != 0){
						
						#ifdef d
							fprintf(stderr,"RUNNING : %s\n",cmd);
						#endif
						if(debug)
							fprintf(stderr,"RUNNING : %s\n",cmd);

						if(execve(newPath,argv,ENVP) < 0){
							printf("Error on executing program %s with error : %s\n",newPath,strerror(errno));
							exit(1);
						}
						
						exit(0);
					}else{
					/*path not found*/
					fprintf(stderr,"%s : command not found\n",argv[0]);
					exit(0x7f);
					}
				}else{
						/*in parent, wait for child to finish*/				
						if(waitpid(-1,&status,0) >= 0){
						/*child successfully reap*/
							if(WIFEXITED(status)){
							/*child terminate correctly*/
								#ifdef d
									fprintf(stderr,"ENDED : %s (ret:%d)\n",cmd,WEXITSTATUS(status));
								#endif
								if(debug)
									fprintf(stderr,"ENDED : %s (ret:%d)\n",cmd,WEXITSTATUS(status));
									return;
							}else{
							/*something wrong when child terminate*/
								fprintf(stderr,"Child terminated abnormally with error:%s\n",strerror(errno));
								return;
							}
						}else{
							fprintf(stderr,"ERROR ON WAITPID with error:%s\n",strerror(errno));
						}
					}
}

int checkRedir(char ** argv){
	int redir = 0;
	/*search through all argv until find > , < or |*/
	char *arg = argv[0];
	int count = 0;
	char *c;
	
	while(arg != NULL){
		c = arg;
		for(int i = 0;i<strlen(arg);i++){
			
			if(*c == '>'){	
			//	printf("OUT\n");
				if(strlen(arg) > 1)
					OUT = parseInt(arg);
				redir = 1;
				direct = '>';
				COUNT = count;
				redirOut(argv,OUT,count);
				}
			else if(*c == '<'){
			//	printf("IN\n");
				if(strlen(arg) > 1)
					IN = parseInt(arg);
				redir = 1;
				direct = '<';
				COUNT = count;
				redirIn(argv,IN,count);
				}
			else if(*c == '|'){
			//	printf("PIPE\n");
				redir = 1;
				redirPipe(argv);
				}
			c++;
		}
		arg = argv[++count];
		
	}
	return redir;
}

void redirOut(char **argv, int out,int count){
	/*direct output of program to fd out*/
	
	fd = OPEN(argv[count+1],O_RDWR|O_CREAT,S_IWUSR|S_IRUSR|S_IXUSR);

	argv[count] = NULL;
	
	exe(argv);

}

void redirIn(char **argv, int in,int count){

	fd = OPEN(argv[count+1],O_RDWR|O_CREAT,S_IWUSR|S_IRUSR|S_IXUSR);

	argv[count] = NULL;
	
	exe(argv);
}

void redirPipe(char **argv){

}

int parseInt(char *arg){

	char *c = arg;
	int value = 0;

	while(*c != '>' && *c != '<'){	
		value = value*10 + ((int)*c - 48);		
		c++;
	}
	//printf("value is %d\n",value);
	return value;
}

void directFile(){
/*for now, assume file name is follow by > or < sign */
	if(direct == '>'){
		
		dup2(fd,OUT);
		direct = 0;
		COUNT = 0;
		OUT = 0;	
		fd = 0;
	}else if(direct == '<'){
		dup2(fd,IN);
		COUNT = 0;
		IN = 0;
		direct = 0;
		fd = 0;
	}		
}
	

int parse(char buf[],char *argv[]){


  
   const char s[1] = " ";
   char *token;
   char *str;
   int index =0;
   /* get the first token */
  
   token = strtok(buf, s);
     
   /* walk through other tokens */
   while( token != NULL ) 
   {    
     argv[index]=token;
     token = strtok(NULL, s);		   
     index++;
   }

	//set last index = NUll	
	argv[index] = NULL;
	
	//get ride of /n 
	str = argv[index-1];
	str = strtok(str, "\n");

	argv[index-1] = str;
  
	//background process condition
	if(argv[index-1] != NULL){
		if(*argv[--index]=='&'){
		argv[index] = NULL;
		return 0;
		}
	}
	//foreground process
	return 1;

	

}

void findPath(char *path,char newPath[]){


 	char fullPath[1028]="";

	int foundPath = 0;
	
	/*if path is absolute or relative, just check if file exist*/
  if(path[0]=='/' || path[0] == '.'){

	if(file_exist(path)){
		strcpy(newPath,path);
	}	
	else{ 
		*newPath = 0;
		}
	}	
	
  /*else find the absolute path*/
  else{

		int i=0;
		while(tokens[i]!=NULL){
		
		
		strcpy(fullPath,tokens[i]);
		strcat(fullPath,"/");
		strcat(fullPath,path);

			if(file_exist(fullPath)){	
				strcpy(newPath,fullPath);
				foundPath = 1;
				break;
			}
			i++;
		}
		
		if(!foundPath){
			*newPath = 0;
			}
	}

	
}

int file_exist (const char *filePath)
{
  struct stat temp;   
  return (stat(filePath, &temp) == 0);
}


void buildIn(char* cmd[], int *build_In){

	if(!strcmp(cmd[0],"exit")){		
		/*exit program*/
		write(1,"PROGRAM EXITING\n",17);
		*build_In = 1;
		 historyFile(1);
		 close(fpp);
		exit(EXIT_SUCCESS);
	}else if(!strcmp(cmd[0],"cd")){		
		/*call cd program*/		
		*build_In = 1;
		CD(cmd);
	}else if(!strcmp(cmd[0],"set")){		
		/*call set program*/
		*build_In = 1;
		SET(cmd);
	}else if(!strcmp(cmd[0],"pwd")){		
		/*call pwd program*/
		*build_In = 1;
		PWD(cmd);
	}else if(!strcmp(cmd[0],"echo")){		
		/*call echo program*/
		*build_In = 1;
		ECHO(cmd);
	}else if(!strcmp(cmd[0],"help")){		
		/*call help program*/
		*build_In = 1;
		HELP(cmd);
	}
}


void CD(char *cmd[0]){
	char *pwd = malloc(100);
	char temp[100];
	char *ret;

	getcwd(pwd,100);
 
	// if cd .. go to previous directory
	if(cmd[1]==NULL){

	strcpy(lastLocation,pwd);
	strcpy( temp,getenv("HOME"));
		if(chdir(temp)<0){
			printf("error: %s\n", strerror(errno));
		}



	}else if(strcmp(cmd[1],"..")==0){
		strcpy(lastLocation,pwd);	

		strcpy(temp,pwd);

		ret= strrchr(temp, '/');

		*ret = '\0'; 
		if(chdir(temp)<0){
			printf("error: %s\n", strerror(errno));
		}

	//if cd - go to last location
	}else if(strcmp(cmd[1],"-")==0){
	
		if(chdir(lastLocation)<0){
			printf("error: %s\n", strerror(errno));
		}
		strcpy(lastLocation,pwd);	
	

	}else if(strcmp(cmd[1],".")==0){
	
    strcpy(lastLocation,pwd); 

	}else if(strncmp(cmd[1],"/",1)==0){
  
    strcpy(lastLocation,pwd); 
    if(chdir(cmd[1])<0){
      printf("error: %s\n", strerror(errno));
    }


  }else{

		strcpy(lastLocation,pwd);	

		strcat(pwd,"/");
		strcat(pwd,cmd[1]);
		if(chdir(pwd)<0){
			printf("error: %s\n", strerror(errno));
		}



	}
	free(pwd);
	
	/*if pwd is changed, reflect on environ*/
	char *PWD = malloc(100);
	getcwd(PWD,100);
	if(strcmp(PWD,getenv("PWD"))){
		if(setenv("PWD",PWD,1))
				fprintf(stderr,"Error in set env with error:%s\n",strerror(errno));
	}
	
	free(PWD);
}

void ECHO(char *cmd[]){
	char c;
	int i = 1;
	char *value;
	char *name;
	/*if argument is NULL, just return*/
	while(cmd[i]!=NULL){
		/*check if argument starts with $, if not, just print the argument*/
		if((c = *cmd[i]) == '$'){
			
			/*search for environ variable, print its value if found, or print arugment otherwise*/
			name = cmd[i];
	
			if(strlen(++name) != 0){
				if((c = *name) == '?'){	
						
					fprintf(stderr,"%d",status>>8);
					if(strlen(++name) != 0){
						write(1,name,strlen(name));
					}
				}else{
					value = getenv(name);
					if(value != NULL)
						write(1,value,strlen(value));
		
					}
			}else{
				write(1,"$",1);
				write(1," ",1);
				}
		}else{
			write(1,cmd[i],strlen(cmd[i]));	
			write(1," ",1);
		}
		i++;
		
	}

	write(1,"\n",1);

}


void SET(char *cmd[0]){

	int valid = 0;
	/*check if set arugments are valid*/

	if(cmd[1] == NULL){
		fprintf(stderr,"Function set should be in format name = value\n");
		return;
		}
	valid++;
	if(cmd[2] != NULL){
	if(strcmp(cmd[2],"=")){
		fprintf(stderr,"Function set should be in format name = value\n");
		return;
		}
	}
	valid++;
	if(cmd[3] == NULL){
		fprintf(stderr,"Function set should be in format name = value\n");
		return;}
	valid++;
		
	if(valid==3){
		/*fprintf(stderr,"name = %s, value = %s\n",cmd[1],cmd[3]);*/
		/*first check if value overlap, if yes concat new value with previous value ans set again*/
		/*char *prev = getenv(cmd[1]);
		if(prev != NULL){
			value already existd
			char newEnv[1024];
			strcpy(newEnv,cmd[3]);
			strcat(newEnv,prev);
			if(setenv(cmd[1],newEnv,1))
				fprintf(stderr,"Error in set env with error:%s\n",strerror(errno));
			fprintf(stderr,"new Path: %s\n",getenv("PATH"));
		}else{*/
			/*value not exist, add new value into env*/
			if(setenv(cmd[1],cmd[3],1))
				fprintf(stderr,"Error in set env with error:%s\n",strerror(errno));
			//fprintf(stderr,"new Path 2: %s\n",getenv(cmd[1]));
		//}
	}
}


void PWD(){
	char *pwd = malloc(100);
	getcwd(pwd,100);
	fprintf(stdout,"%s\n",pwd);
	free(pwd);
}

void splitPath(char *cmd[]){
	char path2[2014];
	char *token;

	int index = 0;
	char *paths = getenv("PATH");
	strcat(path2,paths);

	token = strtok(path2, ":");
	
	//copy all paths to string array
		while( token != NULL ) 
	  	{
	    
		 tokens[index]=token;
		 token = strtok(NULL, ":");
		 index++;
	 }


}

void HELP(){
	fprintf(stdout,"CSE320 SHELL:\n \
	cd					cd dicretory									\n \
	ls					show all file name under current directory		\n \
	set					set environ in format name = value				\n \
	pwd					show current directory							\n \
	echo					print string and expand environment variables	\n \
	help					print help meun									\n");
}

// mode: 1-write 0 - read
void historyFile(int mode){

  
   char str [1024]="";
   int index = 1;
   char*token;
   //char* path = getenv("HOME");
   //strcat(path,"/history.txt");


   //fp = fopen(path, "w+");
   int temp;
   //writing to file
   if (mode == 1){
   		
   		remove(historyLocation);

		fpp = OPEN(historyLocation,O_RDWR|O_CREAT,S_IWUSR|S_IRUSR|S_IXUSR);

   		while(index<50&&strcmp(cmdHistory[index],"")!=0){
   			 strcpy(str,cmdHistory[index]);
   			 strcat(str, "\n");  
   			 
  			 write(fpp,str, strlen(str));
  			 index ++;
   		}
   

   	//reading to buffer
   }else{
   		temp =(read(fpp, str, 1024));
   			if(temp<0){
   				fprintf(stderr,"Read History error: %s\n",strerror(errno));
				return ;
   			}

   		
		token = strtok(str,"\n");

		while(token!=NULL){
	
		strcpy(cmdHistory[index],token);

		token = strtok(NULL,"\n");
		index ++;
			}
		}
   	
   		close(fpp);
   
   

}


void Left(){

    write(1,&cc,1);  
    write(1,&bb,1);
    write(1,"D",1);
}


void Right(){
    write(1,&cc,1);  
    write(1,&bb,1);

    write(1,"C",1);
}

void saveCursor(){
    write(1,&cc,1);  
    write(1,&bb,1);
    write(1,"s",1);
}

void restoreCursor(){
    write(1,&cc,1);  
    write(1,&bb,1);
    write(1,"u",1);
}

void clearLine(){
    write(1,&cc,1);  
    write(1,&bb,1);
    write(1,"K",1);
}

void clearWholeLine(){
    write(1,&cc,1);  
    write(1,&bb,1);
    write(1,"2",1);
    write(1,"K",1);
}


void saveHistory(char* cmd){

	int index =49;
	char*token;
	if(strcmp(cmd,"\n")==0){
		return;
	}
	token = strtok(cmd,"\n");
	//move everything to the right by 1 unit
	while(index !=1){
	strcpy(cmdHistory[index],cmdHistory[index-1]);
	index--;
	}
	strcpy(cmdHistory[1],token);


}

void printPromptDirectory(){
	char *pwd = malloc(100);
  	getcwd(pwd,100);
  	write(1,"[",1);
  	write(1,pwd,strlen(pwd));
  	write(1,"]",1);
  	free(pwd);
}

int OPEN(const char* pathname, int flags, mode_t mode){
	int result;
	result = open(pathname,flags,mode);
	if(result < 0)
		fprintf(stderr,"Open file: %s with error: %s\n",pathname,strerror(errno));
	return result;
}
