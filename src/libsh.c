//
// Used to execute commands
//
//
#include "libsh.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// fork and execute the command
void fork_exec(const char *const args[])
{
    pid_t pid;
    int wstatus;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        execve(args[0], args, NULL);
        perror("execve");
        _exit(1);
    }

    // wait for the command to end
    if (waitpid(pid, &wstatus, 0) < 0) {
        perror("waitpid");
        return;
    }
}

// Partition the command to its parts (seperated by ' ')
// str = the command to execute
// args = the arguments of the command
// nargs = number of arguments
void tokenize_shell_command(char *str, char **args, size_t nargs)
{
    char *tok, *saveptr;
    char **argp;

    for (tok = strtok_r(str, " ", &saveptr), argp = args;
         tok && (argp < args + nargs);
         tok = strtok_r(NULL, " ", &saveptr), argp++) {
        *argp = tok;
    }

    *argp = NULL;
}

// Execute command in a shell
void exec_shell(const char *command)
{
    char *str;
    char *args[1024];

    str = strdup(command);
    if (!str) {
        return;
    }

    // support only 1024 arguments
    // nargs = 1024
    tokenize_shell_command(str, args, sizeof(args) / sizeof(*args));

    fork_exec(args);
    free(str);
}
