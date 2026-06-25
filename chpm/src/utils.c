#include <stdio.h>
#include "chpm.h"

void print_version()
{
    printf("      _                     \n");
    printf("  ___| |__  _ __  _ __ ___  \n");
    printf(" / __| '_ \\| '_ \\| '_ ` _ \\ \n");
    printf("| (__| | | | |_) | | | | | |\n");
    printf(" \\___|_| |_| .__/|_| |_| |_|\n");
    printf("           |_|              \n");
    printf(CHPM_MADE " v" CHPM_VERSION "\n");
    printf("by " CHPM_AUTHOR "\n");
}
