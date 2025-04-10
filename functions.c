#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>


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


void removeDirectoryRecursive(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *entry;
    char filepath[1024];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        struct stat st;
        stat(filepath, &st);
        if (S_ISDIR(st.st_mode)) {
            removeDirectoryRecursive(filepath);
        } else {
            remove(filepath);
        }
    }
    closedir(dir);
    rmdir(path);
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
    if (info->tokenCount == 0) 
        return;

    // // Debugging
    // printf("\n");
    // printf("%s", "token received:\n");
    // for (int i = 0; i < info->tokenCount; i++) {
    //     printf("%s\n", info->tokens[i]);
    // }
    // printf("\n");

    // Switch-case structure for commands
    const char* command = info->tokens[0];
    switch (command[0]) {
        case 'p':  // Commands starting with 'p'
            if (strcmp(command, "pwd") == 0) {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("%s\n", cwd);
                } else {
                    printf("%s", "getcwd() error\n");
                }
            }
            break;

        case 'n':  // Commands starting with 'n'
            if (strcmp(command, "nano") == 0) {
                if (info->tokenCount < 2) {
                    fprintf(stderr, "nano: missing filename\n");
                } else {
                    simulateEditor(info->tokens[1]);
                }
            }
            break;

        case 'c':  // Commands starting with 'c'
            if (strcmp(command, "chmod") == 0) {
                if (info->tokenCount < 3) {
                    printf("Usage: chmod <mode|[ugoa][+-=][rwx]> <file1> [file2] ...\n");
                    break;
                }

                if (strchr("ugoa", info->tokens[1][0]) && strchr("+-=", info->tokens[1][1])) {
                    // Symbolic mode (e.g., g+w)
                    for (int i = 2; i < info->tokenCount; i++) {
                        const char* path = info->tokens[i];
                        struct stat st;
                        if (stat(path, &st) != 0) {
                            perror(path);
                            continue;
                        }

                        mode_t newMode = st.st_mode;
                        char who = info->tokens[1][0];
                        char op = info->tokens[1][1];
                        char perm = info->tokens[1][2];

                        mode_t mask = 0;
                        if (perm == 'r') mask = S_IRUSR | S_IRGRP | S_IROTH;
                        else if (perm == 'w') mask = S_IWUSR | S_IWGRP | S_IWOTH;
                        else if (perm == 'x') mask = S_IXUSR | S_IXGRP | S_IXOTH;

                        if (who == 'u') mask &= S_IRWXU;
                        else if (who == 'g') mask &= S_IRWXG;
                        else if (who == 'o') mask &= S_IRWXO;
                        else if (who == 'a') mask &= S_IRWXU | S_IRWXG | S_IRWXO;

                        if (op == '+') newMode |= mask;
                        else if (op == '-') newMode &= ~mask;
                        else if (op == '=') {
                            if (who == 'u') newMode = (newMode & ~S_IRWXU) | (mask & S_IRWXU);
                            else if (who == 'g') newMode = (newMode & ~S_IRWXG) | (mask & S_IRWXG);
                            else if (who == 'o') newMode = (newMode & ~S_IRWXO) | (mask & S_IRWXO);
                            else if (who == 'a') newMode = mask;
                        }

                        if (chmod(path, newMode) != 0) {
                            perror(path);
                        } else {
                            printf("Changed permissions of '%s'\n", path);
                        }
                    }
                } else {
                    char* endptr;
                    mode_t mode = (mode_t)strtol(info->tokens[1], &endptr, 8);
                    if (*endptr != '\0') {
                        printf("Invalid mode: %s\n", info->tokens[1]);
                        break;
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
            }
            else if (strcmp(command, "clear") == 0) {
                pid_t pid = fork();
                if (pid == 0) {
                    execlp("clear", "clear", (char*)NULL);
                    perror("execlp");
                    exit(1);
                } else {
                    wait(NULL);
                }
            }
            else if (strcmp(command, "cp") == 0) {
                if (info->tokenCount < 3) {
                    printf("Usage: cp <source_file> <destination_file>\n");
                } else {
                    const char* src = info->tokens[1];
                    const char* dest = info->tokens[2];
                    FILE* srcFile = fopen(src, "rb");
                    if (!srcFile) {
                        perror("Source file open error");
                        break;
                    }
                    FILE* destFile = fopen(dest, "wb");
                    if (!destFile) {
                        perror("Destination file open error");
                        fclose(srcFile);
                        break;
                    }
                    char buffer[1024];
                    size_t bytesRead;
                    while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
                        fwrite(buffer, 1, bytesRead, destFile);
                    }
                    printf("Copied '%s' to '%s'\n", src, dest);
                    fclose(srcFile);
                    fclose(destFile);
                }
            }
            else if (strcmp(command, "cat") == 0) {
                if (info->tokenCount < 2) {
                    printf("Usage: cat <filename>\n");
                } else {
                    const char* filename = info->tokens[1];
                    FILE* file = fopen(filename, "r");
                    if (!file) {
                        perror("Error opening file");
                        break;
                    }
                    char buffer[1024];
                    while (fgets(buffer, sizeof(buffer), file)) {
                        printf("%s", buffer);
                    }
                    fclose(file);
                }
            }
            break;

        case 'l':  // Commands starting with 'l'
            if (strcmp(command, "ls") == 0) {
                pid_t pid = fork();
                if (pid == 0) {
                    if (info->tokenCount == 1) {
                        execlp("ls", "ls", (char*)NULL);
                    } else if (info->tokenCount == 2 && strcmp(info->tokens[1], "-l") == 0) {
                        execlp("ls", "ls", "-l", (char*)NULL);
                    } else {
                        fprintf(stderr, "ls: invalid option -- '%s'\n", info->tokens[1]);
                        exit(1);
                    }
                    perror("execlp");
                } else {
                    wait(NULL);
                }
            }
            break;

        case 'g':  // Commands starting with 'g'
            if (strcmp(command, "grep") == 0) {
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
                    execvp("grep", info->tokens);
                    perror("execvp");
                    exit(1);
                } else {
                    wait(NULL);
                }
            }
            break;

        case 'r':  // Commands starting with 'r'
            if (strcmp(command, "rmdir") == 0) {
                if (info->tokenCount < 2) {
                    printf("Usage: rmdir [-r] <directory>\n");
                } else if (strcmp(info->tokens[1], "-r") == 0) {
                    if (info->tokenCount < 3) {
                        printf("Usage: rmdir -r <directory>\n");
                    } else {
                        removeDirectoryRecursive(info->tokens[2]);
                        printf("Directory '%s' removed recursively.\n", info->tokens[2]);
                    }
                } else {
                    if (rmdir(info->tokens[1]) == 0) {
                        printf("Directory '%s' removed successfully.\n", info->tokens[1]);
                    } else {
                        perror("rmdir");
                    }
                }
            } else if (strcmp(command, "rm") == 0) {
                if (info->tokenCount < 2) {
                    printf("Usage: rm [-r] <file|directory>\n");
                } else if (strcmp(info->tokens[1], "-r") == 0) {
                    if (info->tokenCount < 3) {
                        printf("Usage: rm -r <directory>\n");
                    } else {
                        removeDirectoryRecursive(info->tokens[2]);
                        printf("Directory '%s' removed recursively.\n", info->tokens[2]);
                    }
                } else {
                    struct stat st;
                    stat(info->tokens[1], &st);
                    if (S_ISDIR(st.st_mode)) {
                        printf("rm: cannot remove '%s': Is a directory\n", info->tokens[1]);
                    } else {
                        if (remove(info->tokens[1]) == 0) {
                            printf("File '%s' removed successfully.\n", info->tokens[1]);
                        } else {
                            perror("rm");
                        }
                    }
                }
            }
            break;

        case 't':  // Commands starting with 't'
            if (strcmp(command, "tree") == 0) {
                char* args[3];
                args[0] = "./tree";
                args[1] = (info->tokenCount > 1) ? info->tokens[1] : ".";
                args[2] = NULL;
                if (execvp(args[0], args) == -1) {
                    perror("execvp");
                }
                exit(0);
            }
            break;

        default:
            printf("Command '%s' is not recognized in this simple shell version.\n", command);
            break;
    }

    for (int i = 0; i < info->tokenCount; i++)
        free(info->tokens[i]);
    free(info);
}
