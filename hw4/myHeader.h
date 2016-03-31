	
	/**
*Helper function to parse command line into argument list, also determine job type
*@param char buf[]: command line 
*@param char *argv[]: argument list
*@return 0 if background job, foreground otherwise
**/
int parse(char buf[],char *argv[]);



/**
*Helper function to evaluate commnand line
*@param char cmd[]: command line
*@param char** envp: environmental variables
**/
void eva(char cmd[]);

void exe(char**argv);

int FORK();
/**
*Helper function to check if command is build in, if yes, handle them
*@param char* cmd
*@Param int build_In  0 if not build in, build in otherwise
**/
void buildIn(char *cmd[], int *build_In);

/**
*Helper function to find path to binary file
*@param char* file
*@param char newPath[]
*set newFile to NUll is no valid path is found
*/
void findPath(char *file,char newFile[]);

/**
*Helper function using stat to check if file exist
*@param const char *filePath: file to check 
*return true upon success, false otherwise
**/
int file_exist (const char *filePath);

/**
*Helper to build in function CD
*@param char *cmd: command line
**/
void CD(char *cmd[0]);


/**
*Helper to build in function echo
*@param char *cmd: command line
**/
void ECHO(char *cmd[0]);

/**
*Helper to build in function set
*@param char *cmd: command line
**/
void SET(char *cmd[0]);

/**
*Helper to build in function pwd
=
**/
void PWD();

/**
*Helper to build in function help
**/
void HELP();

/**
*Helper to move cursor left 1 position
**/
void Left();

/**
*Helper to move cursor right 1 position
**/
void Right();

/**
*Helper to save current cursor position
**/
void saveCursor();


/**
*Helper to restore  cursor to saved position
**/
void restoreCursor();

/**
*Helper to clean line starting from cursor
**/
void clearLine();

/**
*Helper to clean while line
**/
void clearWholeLine();

/**
*Helper to save each cmd into buffer
**/
void saveHistory(char*cmd);


void printPromptDirectory();


void splitPath(char *cmd[]);

/**
*Helper function: read and write from file history
**/
void historyFile(int mode);

/**
*helper function to find value of specify descriptor
**/
int parseInt(char *arg);

/**helper to check argument if there is redirection
*@param argv: argument list
*@return: 1 if redirection, 0 otherwise
**/
int checkRedir(char **argv);

/**helper to handle redirection >
*@param argv
*@param out
**/
void redirOut(char **argv, int out,int count);

/**helper to handle redirection <
*@param argv
*@param in
**/
void redirIn(char **argv, int in,int count);

/**helper to handle redirection |
*@param argv
**/
void redirPipe(char **argv);

/*Wrapper for open*/
int OPEN(const char* pathname, int flags, mode_t mode);

void directFile();

void EXIT();
