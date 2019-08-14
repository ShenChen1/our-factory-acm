#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *fp = fopen(argv[1], "r");

    fseek(fp, 0, SEEK_END);

    size_t length = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    unsigned char *buf = malloc(length);

    fread(buf, length, 1, fp);

    printf("const unsigned char prog[] = {\n");
    
    int i;
    for (i = 0; i < length; i++)
    {
        if (i && i % 24 == 0)
            printf("\n");

        printf("0x%02x, ", buf[i]);
    }
    
    printf("};\n\n");

    fclose(fp);
}

