#include "chpio.h"
#include "chipojo.h"

void get_file_contents(const char *filepath){


const char *ext = strrchr(filepath, '.');
if (!ext || strcmp(ext, ".chp") != 0)
{
    printf("Error: not found file .chp\n");
    exit(1);
}

FILE *f = fopen(filepath, "rb");
if (!f)
{
    printf("Can't open %s\n", filepath);
    exit(1);
}
fseek(f, 0, SEEK_END);
long size = ftell(f);
fseek(f, 0, SEEK_SET);
input = malloc((size_t)size + 1);
fread(input, 1, (size_t)size, f);
input[size] = '\0';
fclose(f);
}