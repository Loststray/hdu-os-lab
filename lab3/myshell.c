#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define ARG_SIZE 64

void parse_input(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " \t\r\n");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    args[i] = NULL;
}

void execute_command(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("myshell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("myshell");
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

int main(void) {
    char *args[ARG_SIZE];
    char input[BUFFER_SIZE];

    while (1) {
        printf("myshell> ");
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            perror("myshell");
            break;
        }

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        // Parse input into arguments
        parse_input(input, args);

        // Check for built-in commands like "exit"
        if (args[0] == NULL) {
            continue; // Empty command
        }
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "myshell: expected argument to \"cd\"\n");
            } else {
                if (chdir(args[1]) != 0) {
                    perror("myshell");
                }
            }
            continue;
        }
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

        // Execute the command
        execute_command(args);
    }

    return 0;
}
