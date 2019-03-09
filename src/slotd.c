//
// This is the main program that is run remotely.
// Features a simple debugging interface to debug the target machine
// 
#include "liblogin.h"
#include "libcmd.h"
#include "libcfg.h"
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    const struct user *user;

    // Stores the login_msg string
    cfg_set("login_msg", "slot debug console v7.0\n"
                         "unauthorized access is forbidden!\n"
                         "use debug:debug if you're allowed\n");

    // Stores the startup script
    // This script is run in the loop_setup
    cfg_set("startup", "prompt \033[01;32m\\u@\\h\033[00m:\033[01;34m\\w\033[00m$ \n"
                       "echo\n"
                       "echo greetings!\n"
                       "echo good luck lol\n"
                       "echo\n"
                       "exit\n");

    while (1) {
        user = login_do();
        if (!user) {
            break;
        }

        cmd_loop(user);
    }

    return 0;
}
