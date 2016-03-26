	
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
*Helper function to check if command is build in
*@param char* cmd
*@return 0 if not build in, build in otherwise
**/
int buildIn(char cmd[]);

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
