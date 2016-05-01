

void readB(int fd, char arg[]);

int open_clientfd(char * hostname, char * port);

int Getaddrinfo(const char* host,const char*service,const struct addrinfo *hints, struct addrinfo **result);

int Close(int clientfd);

void Select(int n,fd_set *set);

void auditHandler();
void run();
void fileModified();
void colors(char* color);
//printout usage table
void HELP();
//printout all the commands client accepts
void helpCommand();

void serverCommand(int clientfd);

void createLog();
void addLog(char*msg);
void startChat();
//positive = login success o.w. failed
int login(char*host,char*port);

void timeHandler(char* buf);

void logoutHandler();

void listuHandler();

void startChatHandler(char* cmd);

void openChatHandler(char*str);

int stringToInt(char*str);

int findlast();

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

void parseMSG(char*buf,char*msgTo,char*msgFrom,char*msg);

void addChild(int fd,char*user);

void childCommand(int fd);

//return -1 if need to create new else return child fd
int windowCheck(char*user);
/*warppers*/
int Accept(int socket, struct sockaddr *addr, socklen_t *socklen);
int Pthread_join(pthread_t tid, void **thread_return);
int Pthread_detach(pthread_t tid);
int writeV(int fd, char *s,int byte);
/*thread functions*/
void *loginThread(void *vargp);

void* addUser(char *name, void *pair);

/*helper for stdin command*/
void stdinCommand();

/*helper for client command*/
void clientCommand(int listenfd);

void handleError(int error_code,int fd);

int getTime(time_t currentTime);

ssize_t Read(int fd, void*buf,size_t count);

int checkLogin(char *name, int exist);
void sigChild_handler(int sigID);
void removeChild(char* buf);
void users();
void shutDown();
void parseArg(int fd,char arguments[10][1024]);
void *talkThread(void* vargp);
void removeUser(int fd);
void intToS(char *buf,int t);
void accts();
void getHash(void *acct,char *pwd);
int compareHash(char *name, char *pwd);
int Open(const char* pathname, int flags, mode_t mode);
int checkPwd(char *pwd);
void addAcct(char *name, char *pwd);
void initAcct(int fd);
int existUser(void *Cpair, char *name);
int newUser(void *Cpair, char *name);
int promtPwd(char *pwd);
/*for singals */
void sigInt_handler(int sigID);
void cleanUp();

void uoffHandler(char*buffer);
void errorHandler(char*buffer);
