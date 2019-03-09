#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, const char *argv[])
{
    int fd;
    struct sockaddr_un addr;
    char buffer[] = "win\n";

    memset(&addr, 0, sizeof(addr));
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "/var/jackpot.sock", sizeof(addr.sun_path));
    if (connect(fd, &addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return 1;
    }

    if (send(fd, buffer, sizeof(buffer) - 1, 0) < 0) {
        perror("send");
        close(fd);
        return 1;
    }

    close(fd);
    puts("one eyed jack has been jackpotted");

    return 0;
}
