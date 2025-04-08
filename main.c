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

void executePipe(parseInfo* info) {
    int pipefd[2];
    pid_t pid1, pid2;

    int pipeIndex = -1;
    for (int i = 0; i < info->tokenCount; i++) {
        if (strcmp(info->tokens[i], "|") == 0) {
            pipeIndex = i;
            break;
        }
    }

    if (pipeIndex == -1) {
        fprintf(stderr, "Pipe symbol not found in tokens\n");
        return;
    }

    info->tokens[pipeIndex] = NULL;

    char** cmd1 = info->tokens;
    char** cmd2 = &info->tokens[pipeIndex + 1];

    pipe(pipefd);

    if ((pid1 = fork()) == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(cmd1[0], cmd1);
        perror("execvp cmd1");
        exit(1);
    }

    if ((pid2 = fork()) == 0) {
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        execvp(cmd2[0], cmd2);
        perror("execvp cmd2");
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
        for (int i = 0; i < info->tokenCount; i++) {
                    char* token = info->tokens[i];
                    int len = strlen(token);
                    if (token[0] == '"' && token[len - 1] == '"') {
                        token[len - 1] = '\0';
                        memmove(token, token + 1, len - 1);
                    }
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


        int hasPipeToken = 0;
        for (int i = 0; i < info->tokenCount; i++) {
            if (strcmp(info->tokens[i], "|") == 0) {
                hasPipeToken = 1;
                break;
            }
        }

        if (hasPipeToken) {
            executePipe(info);  // Parse and execute pipe command
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


