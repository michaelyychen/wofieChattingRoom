	
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
**/
void eva(char cmd[]);

/**
*Helper function to check if command is build in, if yes, handle them
*@param char* cmd
*@Param int build_In  0 if not build in, build in otherwise
**/
int buildIn(char cmd[], int build_In);

/**
*Helper function to find path to binary file
*@param char* file
*@param char newPath[]
*return 0 if not found, 1 if found
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
void CD(char *cmd);

/**
*Helper to build in function ls
*@param char *cmd: command line
**/
void LS(char *cmd);

/**
*Helper to build in function echo
*@param char *cmd: command line
**/
void ECHO(char *cmd);

/**
*Helper to build in function set
*@param char *cmd: command line
**/
void SET(char *cmd);

/**
*Helper to build in function pwd
*@param char *cmd: command line
**/
void PWD(char *cmd);

/**
*Helper to build in function help
**/
void HELP();

