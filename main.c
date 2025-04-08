/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "functions.h"

#define CMD_BUF_SIZE 1024



void parseCommand(char *cmdLine, char **tokens) {
    char *token = strtok(cmdLine, " \t\n");
    int i = 0;
    while (token != NULL) {
        tokens[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    tokens[i] = NULL;
}

void executePipe(char *cmdLine) {
    char *tokens[MAX_TOKENS];
    parseCommand(cmdLine, tokens);

    int pipefd[2];
    pid_t pid1, pid2;

// create the pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }

    if ((pid1 = fork()) == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
// run the first command
        execvp(tokens[0], tokens);
        perror("execvp");
        exit(1);
    }

    if ((pid2 = fork()) == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
// run the second command
        execvp(tokens[2], &tokens[2]);
        perror("execvp");
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


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
        
        //EXIT
        if (strcmp(info ->tokens[0], "exit") == 0) {
            for (int i=0; i < info -> tokenCount; i++)
                free(info -> tokens[i]);
            free(info);
            printf("%s", "Exiting shell...");
            break;
        }
        
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
        if (strchr(cmdLine, '|')) {
                    executePipe(cmdLine);  // Parse and execute pipe command
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


