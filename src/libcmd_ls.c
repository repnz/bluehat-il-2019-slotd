#include "libsh.h"
#include <stddef.h>

// can execute ls with any argument after --
// this cannot be used for command inject, because it is passed to execve
void cmd_main(char *param)
{
    char *const process[] = {"/bin/ls", "-al", "--", param[0] ? param : NULL, NULL};
    fork_exec(process);
}
