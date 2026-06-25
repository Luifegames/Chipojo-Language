#include "utils.h"
#include "chipojo.h"

void print_version()
{
    printf("  ____ _     _             _       \n");
    printf(" / ___| |__ (_)_ __   ___ (_) ___  \n");
    printf("| |   | '_ \\| | '_ \\ / _ \\| |/ _ \\ \n");
    printf("| |___| | | | | |_) | (_) | | (_) |\n");
    printf(" \\____|_| |_|_| .__/ \\___// |\\___/ \n");
    printf("              |_|       |__/        \n");
    printf("v" CHIPOJO_VERSION " - by " CHIPOJO_AUTHOR " (" CHIPOJO_YEAR ")\n");
}