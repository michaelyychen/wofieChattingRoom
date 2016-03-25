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
