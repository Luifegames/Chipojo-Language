#include "chipojo.h"
#include "io.h"
#include "lexer.h"
#include "native.h"
#include "parser.h"
#include "utils.h"
#include <sys/stat.h>
#include <unistd.h>


extern char *input;
extern int indx;

static int is_directory(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

int main(int argc, char **argv)
{
    indx = 0;
    if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0))
    {
        print_version();
        return 0;
    }

    if (argc < 2)
    {
        printf("Use: %s archive.chp\n", argv[0]);
        return 1;
    }

    // Resolve entry point
    char entry[512];
    if (strcmp(argv[1], ".") == 0)
    {
        FILE *pkg = fopen("package.json", "rb");
        if (pkg)
        {
            fseek(pkg, 0, SEEK_END);
            long sz = ftell(pkg);
            fseek(pkg, 0, SEEK_SET);
            char *json = malloc((size_t)sz + 1);
            fread(json, 1, (size_t)sz, pkg);
            json[sz] = '\0';
            fclose(pkg);
            // Find "main": "value"
            char *m = strstr(json, "\"main\"");
            if (m)
            {
                m = strchr(m + 6, ':');
                if (m)
                {
                    m++;
                    while (*m && *m != '"') m++;
                    if (*m == '"')
                    {
                        m++;
                        int i = 0;
                        while (*m && *m != '"' && i < 500)
                            entry[i++] = *m++;
                        entry[i] = '\0';
                    }
                }
            }
            free(json);
            if (entry[0] == '\0')
                strcpy(entry, "index.chp");
        }
        else
        {
            strcpy(entry, "index.chp");
        }
    }
    else if (is_directory(argv[1]))
    {
        snprintf(entry, sizeof(entry), "%s/main.chp", argv[1]);
    }
    else
    {
        strcpy(entry, argv[1]);
    }

    // chdir to the script's directory so module resolution (./chpm_modules/, ./package.json) works relative to the script
    {
        char *slash = strrchr(entry, '/');
        if (slash)
        {
            *slash = '\0';
            if (entry[0] != '\0')
                chdir(entry);
            char *base = slash + 1;
            get_file_contents(base);
        }
        else
        {
            get_file_contents(entry);
        }
    }
    register_natives();
    jumpBOM();
    forward();
    program();
    free(input);

    return 0;
}