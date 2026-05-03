#include "lizard.h"
#include "lexer.h"
#include "parser.h"

extern char *input;
extern int indx;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Use: %s archivo.lzd\n", argv[0]);
        return 1;
    }

    // Check extension .lzd
    char *ext = strrchr(argv[1], '.');
    if (!ext || strcmp(ext, ".lzd") != 0)
    {
        printf("Error: not found file .lzd\n");
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f)
    {
        printf("Can't open %s\n", argv[1]);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    input = malloc(size + 1);
    fread(input, 1, size, f);
    input[size] = '\0';
    fclose(f);

    indx = 0;
    jumpBOM();
    forward();
    program();
    free(input);

    return 0;
}