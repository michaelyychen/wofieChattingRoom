


int open_clientfd(char * hostname, char * port);

int Getaddrinfo(const char* host,const char*service,const struct addrinfo *hints, struct addrinfo **result);

int Close(int clientfd);

void Select(int n,fd_set *set);

void HELP();






/*
hua li de fen ge xian
--------------------------------------------------------------------------------
 */

/*helper to open a listen fd
return listenfd upon success, -1 otherwise
*/ 
int open_listenfd(char * port);

void color(char* color);

/*helper for stdin command*/
void stdinCommand();

/*helper for client command*/
void clientCommand(int listenfd);

void getTime(time_t current_time);
