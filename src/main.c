#include "lizard.h"
#include "io.h"
#include "lexer.h"
#include "native.h"
#include "parser.h"
#include "utils.h"
#include "core.h"
#include "input.h"


extern char *input;
extern int indx;

int main(int argc, char **argv)
{
    indx = 0;
    if (argc == 2 && strcmp(argv[1], "-v") == 0)
    {
        print_version();
        return 0;
    }

    if (argc < 2)
    {
        printf("Use: %s archive.lzd\n", argv[0]);
        return 1;
    }
    
    get_file_contents(argv[1]);
    register_natives();
    jumpBOM();
    forward();
    program();
    if (function_exist("_draw")){
        register_input();
        run_game();
    }
    free(input);

    return 0;
}