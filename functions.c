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
        return;
    }

    //LS
    if (strcmp(info -> tokens[0], "ls") == 0) {
        if (info->tokenCount == 1) {
            pid_t pid = fork();
            if (pid == 0) {
                // Child process: Execute the `ls` command
                execlp("ls", "ls", (char*)NULL);
                perror("execlp");
            } else {
                wait(NULL);
            }
        } else if (info->tokenCount == 2 && strcmp(info->tokens[1], "-l") == 0) {
            pid_t pid = fork();
            if (pid == 0) {
                // Child process: Execute the `ls -l` command
                execlp("ls", "ls", "-l", (char*)NULL);
                perror("execlp");
            } else {
                wait(NULL);
            }
        } else {
            fprintf(stderr, "ls: invalid option -- '%s'\n", info->tokens[1]);
            return;
        }
        return;
    }
    // CLEAR
    if (strcmp(info -> tokens[0], "clear") == 0) {
        // Execute the `clear` command
        pid_t pid = fork();
        if (pid == 0) {
            // Child process: Execute the `clear` command
            execlp("clear", "clear", (char*)NULL);
            perror("execlp");  // If execlp fails, it will print this error
            exit(1);
        } else {
            // Parent process: Wait for the child to finish
            wait(NULL);
        }
    }
    // GREP
    if (strcmp(info -> tokens[0], "grep") == 0&& strcmp(info->tokens[1], "-c") != 0) {
        for (int i = 0; i < info->tokenCount; i++) {
            char* token = info->tokens[i];
            int len = strlen(token);
            if (token[0] == '"' && token[len - 1] == '"') {
                token[len - 1] = '\0';
                memmove(token, token + 1, len - 1);
            }
        }
        pid_t pid = fork();
        if (pid == 0) {
            // Child process: Execute the `grep` command
            execvp("grep", info->tokens);  // info->tokens should hold the arguments passed to `grep`
            perror("execvp");
            exit(1);
        } else {
            // Parent process: Wait for the child to finish
            wait(NULL);
        }
        return;
    }


    // GREP -C
    if (strcmp(info -> tokens[0], "grep") == 0 && strcmp(info->tokens[1], "-c") == 0) {
        for (int i = 0; i < info->tokenCount; i++) {
            char* token = info->tokens[i];
            int len = strlen(token);
            if (token[0] == '"' && token[len - 1] == '"') {
                token[len - 1] = '\0';
                memmove(token, token + 1, len - 1);
            }
        }
        pid_t pid = fork();
        if (pid == 0) {
            // Child process: Execute the `grep -c` command
            execvp("grep", info->tokens);  // info->tokens should hold the arguments passed to `grep -c`
            perror("execvp");
            exit(1);
        } else {
            // Parent process: Wait for the child to finish
            wait(NULL);
        }
        return;
    }

    // CP (copy file)
    if (strcmp(info->tokens[0], "cp") == 0) {
        if (info->tokenCount < 3) {
            printf("Usage: cp <source_file> <destination_file>\n");
            return;
        }

        const char* src = info->tokens[1];
        const char* dest = info->tokens[2];

        FILE* srcFile = fopen(src, "rb");
        if (!srcFile) {
            perror("Source file open error");
            return;
        }

        FILE* destFile = fopen(dest, "wb");
        if (!destFile) {
            perror("Destination file open error");
            fclose(srcFile);
            return;
        }

        char buffer[1024];
        size_t bytesRead;

        while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
            fwrite(buffer, 1, bytesRead, destFile);
        }

        printf("Copied '%s' to '%s'\n", src, dest);

        fclose(srcFile);
        fclose(destFile);
        return;
    }

    else {
        printf("Command '%s' is not recognized in this simple shell version.\n", info->tokens[0]);
    }
    
    for (int i=0; i < info -> tokenCount; i++)
        free(info -> tokens[i]);
    free(info);
}
