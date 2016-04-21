
int open_clientfd(char * hostname, char * port);

int Getaddrinfo(const char* host,const char*service,const struct addrinfo *hints, struct addrinfo **result);

int Close(int clientfd);

void Select(int n,fd_set *set);

//printout usage table
void HELP();
//printout all the commands client accepts
void helpCommand();

void serverCommand(int clientfd);

void startChat();
//positive = login success o.w. failed
int login();

void timeHandler(char* buf);

void logoutHandler();

void listuHandler();

void startChatHandler(char* cmd);

void openChatHandler(char*str);

int stringToInt(char*str);

void listuHandler(char* buffer);
/*
hua li de fen ge xian
--------------------------------------------------------------------------------
 */

/*helper to open a listen fd
return listenfd upon success, -1 otherwise
*/
int open_listenfd(char * port);


void errorPrint();

void color(char*color,int fd);


/*warppers*/
int Accept(int socket, struct sockaddr *addr, socklen_t *socklen);
int Pthread_join(pthread_t tid, void **thread_return);
int Pthread_detach(pthread_t tid);

/*thread functions*/
void *loginThread(void *vargp);


/*helper for stdin command*/
void stdinCommand();

/*helper for client command*/
void clientCommand(int listenfd);

void handleError(int error_code,int *fd);
void getTime(time_t current_time);

ssize_t Read(int fd, void*buf,size_t count);

int checkLogin(char *name);
