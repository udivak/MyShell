#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   
#include <sys/stat.h> 

void print_tree(const char* base_path, int level) {
    DIR* dir = opendir(base_path);
    if (dir == NULL) {
        printf("%s", "Error opendir");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip the current and parent directory entries.
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Print indentation based on the level.
        for (int i = 0; i < level; i++) {
            printf("/-----");
        }

        // Construct the full path for the current entry.
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        // Use stat to check if this is a directory
        struct stat path_stat;
        if (stat(path, &path_stat) == -1) {
            printf("%s", "Error stat");
            continue;
        }

        // Check if entry is a directory.
        if (S_ISDIR(path_stat.st_mode)) {
            // Print the directory name.
            printf("[DIR] %s\n", entry->d_name);
            // Recursively print the subdirectory contents.
            print_tree(path, level + 1);
        }
        else {
            // It is a file, so simply print the file name.
            printf("%s\n", entry->d_name);
        }
    }
    closedir(dir);
}


int main(int argc, char **argv) {
    char start_path[1024];

    // Use the first command line argument if provided, else use the current directory "."
    if (argc > 1) {
        strncpy(start_path, argv[1], sizeof(start_path) - 1);
        start_path[sizeof(start_path) - 1] = '\0';
    } else {
        strcpy(start_path, ".");
    }

    printf("Directory tree for %s:\n", start_path);
    print_tree(start_path, 0);

    return 0;
}

