#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"
#include <sys/types.h>
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

void simulateEditor(char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("%s", "Error fopen");
        exit(1);
    }
    printf("Simulated editor opened '%s'.\n", filename);
    printf("Type your text below. Type 'EOF' on a new line to finish editing.\n");
    char line[1024];
    while (1) {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;  // In case of end-of-file.
        }
        // If the line equals "EOF" (with or without newline), finish editing.
        if (strcmp(line, "EOF\n") == 0 || strcmp(line, "EOF") == 0) {
            break;
        }
        fputs(line, fp);
    }
    
    fclose(fp);
    printf("Finished editing '%s'.\n", filename);
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
    else if (strcmp(info -> tokens[0], "nano") == 0) {
        if (info->tokenCount < 2) {
            fprintf(stderr, "nano: missing filename\n");
        } else {
            simulateEditor(info->tokens[1]);
        }
    }
    
    else if (strcmp(info->tokens[0], "cat") == 0 && info->tokenCount >= 3 && strcmp(info->tokens[1], ">") == 0) {
        simulateEditor(info->tokens[2]);
    }
    
    else {
        printf("Command '%s' is not recognized in this simple shell version.\n", info->tokens[0]);
    }
    
    for (int i=0; i < info -> tokenCount; i++)
        free(info -> tokens[i]);
    free(info);
}

