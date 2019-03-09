#include "libsh.h"
#include <stddef.h>

void cmd_main(char *param)
{
	// wtf.. mount?
    exec_shell("/bin/mount");
}
