#include "libuser.h"
#include "liblogin.h"
#include "libkv.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *const delimiters = " \t";

// enable = 1
// disable = 0
// other = -1
static int parse_enable_disable(const char *token)
{
    if (strcmp("enable", token) == 0) {
        return 1;
    }

    if (strcmp("disable", token) == 0) {
        return 0;
    }

    return -1;
}

// print current username
static int cmd_default(char **tokenizer)
{
    const char *user = user_current();

    if (!user) {
        return 0;
    }

    puts(user);
    return 0;
}


// add a user to the store
static int cmd_add(char **tokenizer)
{
    const char *token, *name, *pw;
    int enable = 1;

    name = strtok_r(NULL, delimiters, tokenizer);
    if (!name) {
        return -1;
    }

    token = strtok_r(NULL, delimiters, tokenizer);
    if (!token) {
        return -1;
    }

    if (strcmp(token, "pw") != 0) {
        return -1;
    }

    pw = strtok_r(NULL, delimiters, tokenizer);
    if (!pw) {
        return -1;
    }

    token = strtok_r(NULL, delimiters, tokenizer);
    if (token) {
        enable = parse_enable_disable(token);
        if (enable < 0) {
            return -1;
        }
    }

    if (user_add(name, pw) < 0) {
        fprintf(stderr, "failed to add user %s\n", name);
        return 0;
    }

    if (enable && (login_enable(user_get(name)) < 0)) {
        fprintf(stderr, "failed to enable user %s\n", name);
    }

    return 0;
}

static int cmd_set(char **tokenizer)
{
    const char *token, *name, *pw = NULL;
    int enable = -1;

    name = strtok_r(NULL, delimiters, tokenizer);
    if (!name) {
        return -1;
    }

    token = strtok_r(NULL, delimiters, tokenizer);
    if (!token) {
        return -1;
    }

    if (token && strcmp(token, "pw") == 0) {
        pw = strtok_r(NULL, delimiters, tokenizer);
        if (!pw) {
            return -1;
        }

        token = strtok_r(NULL, delimiters, tokenizer);
    }

    if (token) {
        enable = parse_enable_disable(token);
        if (enable < 0) {
            return -1;
        }
    }

    if (pw) {
        if (user_set(name, pw) < 0) {
            fprintf(stderr, "failed to update user %s\n", name);
            return 0;
        }
    }

    if (enable >= 0) {
        if (enable) {
            if (login_enable(user_get(name)) < 0) {
                fprintf(stderr, "failed to enable user %s\n", name);
            }
        } else {
            login_disable(user_get(name));
        }
    }

    return 0;
}

static int cmd_del(char **tokenizer)
{
    char *name;
    const struct user *user;

    name = strtok_r(NULL, delimiters, tokenizer);
    if (!name) {
        return -1;
    }

    user = user_get(name);
    if (!user) {
        fprintf(stderr, "no such user\n");
        return 0;
    }

    login_disable(user);

    if (user_del(name) < 0) {
        fprintf(stderr, "failed to remove user\n");
    }

    return 0;
}

static const struct command {
    const char *const cmd;
    const char *const help;
    int (*func)(char **tokenizer);
} commands[] = {
    {"", "", cmd_default},
    {"add", "add <name> pw <pw> [<enable|disable>]", cmd_add},
    {"set", "set <name> [pw <pw>] [<enable|disable>]", cmd_set},
    {"del", "del <name>", cmd_del},
    {NULL, NULL, NULL},
};

static void help(void)
{
    puts("usage:");
    for (const struct command *cp = commands; cp->func; cp++) {
        printf("  user %s\n", cp->help);
    }
}

// find the user command from the list
static const struct command *find_cmd(const char *str)
{
    if (!str) {
        return &commands[0];
    }

    for (const struct command *cp = commands; cp->func; cp++) {
        if (strcmp(str, cp->cmd) == 0) {
            return cp;
        }
    }

    return NULL;
}

void cmd_main(char *param)
{
    char *saveptr = NULL;
    char *tok;
    const struct command *cmd;

    tok = strtok_r(param, delimiters, &saveptr);
    cmd = find_cmd(tok);

    if (!cmd) {
        help();
        return;
    }

    if (cmd->func(&saveptr) < 0) {
        help();
    }
}
