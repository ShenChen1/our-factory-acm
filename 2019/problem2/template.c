#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
    const char path[] = "/var/tmp/prog";
    int fd = creat(path, S_IRWXU);
    write(fd, prog, sizeof(prog));
    close(fd);

    char cmdline[512];
    snprintf(cmdline, sizeof(cmdline), "%s %s", path, argv[1]);
    system(cmdline);
}
