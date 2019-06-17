#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    const char path[] = "/var/tmp/prog";
    FILE *fp = fopen(path, "w");
    fwrite(prog, sizeof(prog), 1, fp);
    fclose(fp);

    char cmdline[512];
    snprintf(cmdline, sizeof(cmdline), "chmod +x %s;%s %s;rm %s", path, path, argv[1], path);
    system(cmdline);
}
