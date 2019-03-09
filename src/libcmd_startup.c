#include "libcfg.h"
#include <stdio.h>
#include <string.h>

static void set_script(void)
{
    // static buffer
    // overflow on this buffer allows many things..?
    static char buffer[4096];
    size_t buffill;
    char *line = NULL;
    size_t linesz = 0;
    size_t len;

    puts("reading commands");

    buffer[0] = '\0';

    // total length of the buffer
    buffill = 1;

    while (1) {
        // read line 
        len = getline(&line, &linesz, stdin);
        if (len <= 0) {
            break;
        }

        // PROBLEM: overflow?
        // this checks that the data won't overflow 
        // Allows to remove the NULL terminator..?
        if ((len + buffill) > (sizeof(buffer) - 1)) {
            break;
        }

        // buffer += line;
        strncat(buffer, line, sizeof(buffer) - 1 - buffill);
        buffill += len;

        if (strcmp("exit\n", line) == 0 || strcmp("quit\n", line) == 0) {
            break;
        }
    }

    cfg_set("startup", buffer);
}

void cmd_main(char *param)
{
    const char *script;

    // if param is set call set_script
    // else print startup script
    if (strcmp(param, "set") == 0) {
        set_script();
        return;
    }

    script = cfg_get("startup");
    if (!script) {
        return;
    }

    printf("%s", script);
}
