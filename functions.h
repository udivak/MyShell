#define FUNCS_H
// Maximum number of tokens the shell can handle.
// You can adjust this as needed.
#define MAX_TOKENS 10
#define MAX_TOKEN_SIZE 100

typedef struct {
    char* tokens[MAX_TOKENS];  // Array of token strings.
    int tokenCount;            // Number of tokens found.
} parseInfo;

parseInfo* parse(char* cmdLine);
void executeCommand(parseInfo* command);
void simulateEditor(char* filename);