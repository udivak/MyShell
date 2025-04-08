#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

parseInfo* parse(char* cmdLine) {
    parseInfo* info = malloc(sizeof(parseInfo));
    if (!info) {
        printf("%s", "Memory Allocation Error");
        exit(1);
    }
    info -> tokenCount = 0;
    
    char* token = strtok(cmdLine, " \t\n");
    while (token != NULL && info->tokenCount<MAX_TOKENS) {
        int tokenLen = strlen(token);
        if (tokenLen > 0 && token[tokenLen - 1] == '\n') 
            token[tokenLen - 1] = '\0';
        info -> tokens[info -> tokenCount] = strdup(token);
        info -> tokenCount++;
        token = strtok(NULL, " ");
    }
    return info;
}

void executeCommand(parseInfo* info) {
    if (info -> tokenCount == 0) 
        return;
    
    //debugging
    printf("\n");
    printf("%s", "token recieved :\n");
    for (int i=0; i<info -> tokenCount; i++){
        printf("%s\n",info->tokens[i]);
    }
    printf("\n");
    //
    
    //PWD
    if (strcmp(info -> tokens[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        }
        else {
            printf("%s", "getcwd() error\n");
        }
    }
    
    //NANO
    if (strcmp(info -> tokens[0], "nano") == 0) {
        
    }
    //CHMOD
    if (strcmp(info->tokens[0], "chmod") == 0) {
        if (info->tokenCount < 3) {
            printf("Usage: chmod <mode> <file1> [file2] ...\n");
            return;
        }

        char* endptr;
        mode_t mode = (mode_t) strtol(info->tokens[1], &endptr, 8); //base 8

        if ( *endptr != '\0') {
            printf("Invalid mode: %s\n", info->tokens[1]);
            return;
        }

        for (int i = 2; i < info->tokenCount; i++) {
            const char* path = info->tokens[i];
            if (chmod(path, mode) != 0) {
                perror(path);
            } else {
                printf("Changed permissions of '%s'\n", path);
            }
        }


    }
    else {
        printf("Command '%s' is not recognized in this simple shell version.\n", info->tokens[0]);
    }
    
    for (int i=0; i < info -> tokenCount; i++)
        free(info -> tokens[i]);
    free(info);
}

