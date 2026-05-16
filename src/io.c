#include "io.h"
#include "lizard.h"

void get_file_contents(const char *filepath){


char *ext = strrchr(filepath, '.');
if (!ext || strcmp(ext, ".lzd") != 0)
{
    printf("Error: not found file .lzd\n");
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
input = malloc(size + 1);
fread(input, 1, size, f);
input[size] = '\0';
fclose(f);
}