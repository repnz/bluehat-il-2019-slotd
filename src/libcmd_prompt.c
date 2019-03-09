#include "libkv.h"
#include "libuser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// Replace certain escape chars 
// \w: replace with working directory
// \u: replace with current user
// \h: replace with 'slotd'
// return a new string
static char *fill_special(char *str)
{
    char *old = NULL, *new = NULL;
    const char *rep;
    char *tok, *saveptr;
    char *dir = NULL;
    char buf[2] = {0, 0};

    // allocate an empty string
    old = strdup("");

    if (!old) {
        return NULL;
    }

    new = old;

    // Escape the PS1 string
    for (tok = strtok_r(str, "\\", &saveptr);
         tok;
         tok = strtok_r(NULL, "\\", &saveptr)) {

        // check the escape character
        switch (tok[0]) {
            case 'u': // user
                rep = user_current();
                break;
            case 'h': // slotd
                rep = "slotd";
                break;
            case 'w': // working directory
                if (!dir) {
                    dir = get_current_dir_name();
                }
                rep = dir;
                break;
            default:
                buf[0] = tok[0];
                rep = buf;
                break;
        }

        new = NULL;
        if (asprintf(&new, "%s%s%s", old, rep, (tok[0] != '\0' ? tok + 1 : "")) < 0) {
            free(new);
            new = NULL;
            break;
        }

        free(old);
        old = new;
    }

    if (old != new) {
        free(old);
        old = NULL;
    }

    if (dir) {
        free(dir);
    }

    return new;
}

void cmd_main(char *param)
{
    char **ref;

    // get ref to env:PS1
    ref = (char **)kv_ref("env", "PS1", free);
    if (!ref) {
        return;
    }

    if (param[0] == '\0') {
        puts(*ref ? *ref : "(no prompt set)");
        return;
    }

    // PROBLEM: why freeing when before you check for NULL..?
    // Maybe can be used for double free
    free(*ref);
    *ref = fill_special(param);
}
