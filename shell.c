#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#define bufSize 512

void parse(char* buffer);
char* trim_space(char *in);
int split(char* str, char* delim, char** args);
void execute(char* input, int* flag);
int is_empty(const char *s);

int main(int argc, char** argv){

    if (argc == 1){
        while (1){
            char buffer[bufSize];
            printf("Pavlidis_9015> ");      // Print prompt
            fgets(buffer,bufSize,stdin);
            if (is_empty(buffer)){          // If input is empty dont crash, continue
                printf("Enter something\n");
                continue;
            }
            char* trimed = trim_space(buffer);  
            if (strcmp(trimed,"quit") == 0){        // Quit even if the string contains whitespaces
                printf("Exiting...\n");
                exit(0);
            }
            // printf("Output: %s", buffer);
            parse(trimed);      // Parse input
        }
    }
    else if (argc == 2){
        FILE* fp;
        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        fp = fopen(argv[1], "r");       // Open desired batchfile if it exists
        if (fp == NULL){
            printf("Could not open file\n");
            exit(EXIT_FAILURE);
        }

        while ((read = getline(&line, &len, fp)) != -1) {
            if (is_empty(line) || (strstr(line,"#") != NULL)){      // Skip empty lines and comments
                continue;
            }
            char* trimed = trim_space(line);
            if (strcmp(trimed,"quit") == 0){
                printf("Exiting...\n");
                exit(0);
            }
            // printf("Output: %s", line);
            parse(trimed);
        }

        fclose(fp);
    }
    else{
        printf("Invalid number of inputs\n");
    }

    return 0;
}

// Function that parses the input of the shell
void parse(char* buffer){
    // printf("String inside func: %s\n", buffer);
    char *testAmp, *testSemCol, *testPip;
    char** args = malloc(bufSize*sizeof(char*));
    int i, flag;
        
    if ((testPip = strstr(buffer,"|")) != NULL){
        printf("Current version of the shell does not support pipes\n");
        return;
    }
    
    if ((testSemCol = strchr(buffer,';')) != NULL){
        if ((testSemCol = strstr(buffer,";;")) != NULL){        // Check if there are 2 or more semicolons
            printf("Syntax Error\n");
            return;
        }
        testSemCol = strstr(buffer,";");
        if (strcmp(testSemCol,buffer) == 0){       // Check if the line starts with semicolon
            printf("Syntax Error\n");
            return;
        }
        i = split(buffer, ";", args);           // Split commands
        args = realloc(args,i*sizeof(char*));   // Resize array to save memory
        // for (int j=0; j<i; j++){
        //     printf("Arguments: %s\n",args[j]);
        // }
        for (int j=0; j<i; j++){
            execute(args[j],&flag);     // Execute each command
        }
    }
    else if ((testAmp = strstr(buffer,"&&")) != NULL){
        if ((testAmp = strstr(buffer,"&&&")) != NULL){      // Check if there are 3 or more ampersands
            printf("Syntax Error\n");
            return;
        }
        testAmp = strstr(buffer,"&&");
        if (strcmp(testAmp,buffer) == 0){       // Check if the line starts with ampersand
            printf("Syntax Error\n");
            return;
        }
        i = split(buffer, "&&", args);              // Split commands
        args = realloc(args,i*sizeof(char*));       // Resize array to save memory
        // for (int j=0; j<i; j++){
        //     printf("Arguments: %s\n",args[j]);
        // }
        for (int j=0; j<i; j++){
            execute(args[j],&flag);     // Execute each command
            if (flag == 10){
                break;          // If one command fails, we dont want the rest to get executed
            }
        }
    }
    else{
        execute(trim_space(buffer),&flag);
    }
}

// Function to execute a simple command or a command with redirection
void execute(char* input, int* flag){
    char** com = malloc(bufSize*sizeof(char*));
    char* testRed1, *testRed2;
    
    if (((testRed1=strchr(input,'>')) == NULL) && ((testRed2=strchr(input,'<')) == NULL)){      // Check if we have no redirection
        int i = split(input, " ", com);     // Split command and parameters
        com = realloc(com,i*sizeof(char*));     // Resize array to save memory
        for (int j=0; j<i; j++){
            // printf("Command: %s\n",com[j]);
        }
    }
    
    pid_t pid;
    int status,fd_in,fd_out;
    pid = fork();       // Create child process to execute the command

    if (pid == 0){
       
        if (testRed1 != NULL){
            int i = split(input, ">", com);     // Split command and file based on redirect symbol
            char** com1 = malloc(bufSize*sizeof(char*));
            int k = split(com[0]," ", com1);      // Split command and parameters of the first part
            com1 = realloc(com1,k*sizeof(char*));
            for (int j=0; j<k; j++){
                // printf("Command: %s\n",com1[j]);
            }
            fd_out = creat(com[1], 0644);       // Create the file in which we will redirect the output
            if (fd_out == -1){
                printf("File failed to get created, errno = %d\n", errno);
                _exit(1);
            }
            dup2(fd_out,STDOUT_FILENO);     // Redirect output
            close(fd_out);

            if (execvp(com1[0], com1) == -1) {                  // Execute command and exit
                printf("Error during command execution\n");
            }
            _exit(10);       // Exit from child process if execution fails
        }

        if (testRed2 != NULL){
            int i = split(input, "<", com);     // Split command and file based on redirect symbol
            char** com1 = malloc(bufSize*sizeof(char*));
            int k = split(com[0]," ", com1);    // Split command and parameters of the first part
            com1 = realloc(com1,k*sizeof(char*));
            for (int j=0; j<k; j++){
                // printf("Command: %s\n",com1[j]);
            }
            fd_in = open(com[1], O_RDONLY);     // Open the file from which we will take the input
            if (fd_in == -1){
                printf("File failed to get opened, errno = %d\n", errno);
                _exit(1);
            }
            dup2(fd_in,STDIN_FILENO);       // Redirect input
            close(fd_in);

            if (execvp(com1[0], com1) == -1) {              // Execute command and exit
                printf("Error during command execution\n");
            }
            _exit(10);       // Exit from child process if execution fails

        }

        if (execvp(com[0], com) == -1) {                // Execute command and exit
            printf("Error during command execution\n");
        }
        _exit(10);       // Exit from child process if execution fails
    }
    else if (pid < 0){
        printf("Fork has failed");
    }
    else{
        wait(&status);          // Parent waits for child
        *flag = status/255;     // Flag based on exit code from child. Divide with 255 to get the desired arithmetic value (10)
    }
}

// Function to split a string and store each substring in array. Returns array size
int split(char* str, char* delim, char** args){
    int i = 0;
    char* token = strtok(str, delim);
    while (token != NULL) {
        // printf("%s", token);
        if (is_empty(token)){
            break;
        }
        args[i++] = trim_space(token);
        token = strtok(NULL, delim);   
    }
    return i;
}


// Function to trim white-spaces arround a string
char *trim_space(char *in)
{
    char *out = NULL;
    int len;
    if (in) {
        len = strlen(in);
        while(len && isspace(in[len - 1])) --len;
        while(len && *in && isspace(*in)) ++in, --len;
        if (len) {
            out = strndup(in, len);
        }
    }
    return out;
}

// Function to check if a string contains only white-spaces ("","\n","\r",etc)
int is_empty(const char *s) {
  while (*s != '\0') {
    if (!isspace((unsigned char)*s)){
        return 0;
    }
    s++;
  }
  return 1;
}