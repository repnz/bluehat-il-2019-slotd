//
// This utility is used to print the current IP
// Uses the libsh module

#include "libsh.h"
#include <stddef.h>

void cmd_main(char *param)
{
    exec_shell("/bin/ip addr");
}
