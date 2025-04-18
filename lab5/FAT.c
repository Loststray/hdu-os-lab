#include <fs.c>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/** List of builtin commands, followed by their corresponding functions. */
char *builtin_str[] = {"format", "cd",    "mkdir", "rmdir", "ls",
                       "create", "rm",    "write", "read",  "exit",
                       "open",   "close", "pwd"};

int (*builtin_func[])(char **) = {
    &my_format, &my_cd,    &my_mkdir, &my_rmdir, &my_ls,
    &my_create, &my_rm,    &my_write, &my_read,  &my_exit_sys,
    &my_open,   &my_close, &my_pwd};

int csh_num_builtins(void) { return sizeof(builtin_str) / sizeof(char *); }
/*
 * @brief Launch a program and wait for it to terminate
 * @param args Null terminated list of arguments.
 * @return Always return 1, to continue executing.
 */
int csh_launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("csh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("csh");
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/*
 * @brief Execute shell built-in or launch program.
 * @param args Null terminated list of arguments.
 * @return 1 if the shell should continue running, 0 if it should terminate.
 */
int csh_execute(char **args) {
    int i;
    if (args[0] == NULL) {
        // An empty command was entered
        return 1;
    }

    for (i = 0; i < csh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return csh_launch(args);
}

/*
 * @brief Read a line of input from stdin.
 * @return The line from stdin.
 */
char *csh_read_line(void) {
    char *line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

#define CSH_TOK_BUFSIZE 64
#define CSH_TOK_DELIM " \t\r\n\a"
/*
 * @brief Split a line into tokens.
 * @param line The line.
 * @return Null-terminated array of tokens.
 */
char **csh_split_line(char *line) {
    size_t bufsize = CSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens) {
        fprintf(stderr, "csh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, CSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += CSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "csh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, CSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/*
 * @brief Loop getting input and execting it.
 */
void csh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("\n\e[1mfiatiustitia\e[0m@XZL \e[1m%s\e[0m\n", current_dir);
        printf("> \e[032m$\e[0m ");
        line = csh_read_line();
        args = csh_split_line(line);
        status = csh_execute(args);

        free(line);
        free(args);
    } while (status);
}

/*
 * @brief Main entry point.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return status code.
 */
int main(void) {
    start_sys();
    csh_loop();

    return EXIT_SUCCESS;
}
