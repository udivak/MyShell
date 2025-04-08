/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "functions.h"

#define CMD_BUF_SIZE 1024

int main(int argc, char **argv) {
    char cmdLine[CMD_BUF_SIZE];
    pid_t childPid;
    parseInfo* info;

    while (1) {
        // Display prompt and get user input.
        printf("> ");
        if (fgets(cmdLine, sizeof(cmdLine), stdin) == NULL) {
            // Handle end-of-file (Ctrl+D)
            printf("\n");
            break;
        }

        // Ignore empty commands
        if(strlen(cmdLine) <= 1) {
            continue;
        }
        
        // Parse the input command into tokens.
        info = parse(cmdLine);
        
        // Parent Process functions
        // CD
        if (strcmp(info -> tokens[0], "cd") == 0) {
            if (info -> tokenCount == 1) {
                printf("%s", "cd with no path requested\n");
                chdir("/Users/");      //base dir
            }
            else {
                char* new_cd = info -> tokens[1];
                if (chdir(info->tokens[1]) != 0) {
                    printf("%s", "chdir error\n");
                    printf("%s", info->tokens[1]);
                }
            }
        }
        else {
            // Create a child process
            childPid = fork();
            if (childPid < 0) {
                printf("fork error");
                exit(1);
            }
            if (childPid == 0) {
                // Child process: execute command.
                executeCommand(info);
                // Exit after command execution.
                exit(0);
            } else {
                // Parent process: wait for the child to finish.
                waitpid(childPid, NULL, 0);
            }
        }
    }
    return 0;
}
