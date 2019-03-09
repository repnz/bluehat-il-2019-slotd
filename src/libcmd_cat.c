//
// This module reads data from a file and prints the data
// Any file can be opened
//
#include <stdio.h>


void cmd_main(char *param)
{
    FILE *fp;
    char buf[1024];
    size_t size;

    // open file
    fp = fopen(param, "rb");
    if (!fp) {
        perror("failed to open file");
        return;
    }

    // while file has data
    while (!feof(fp)) {
        // read data from the file
        size = fread(buf, 1, sizeof(buf), fp);
        if (ferror(fp)) {
            perror("read");
            break;
        }
        // write the data to stdout
        fwrite(buf, 1, size, stdout);
    }
    // close the file
    fclose(fp);
}
