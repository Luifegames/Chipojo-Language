#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include "chpm.h"
#include "utils.h"

#define MAX_DEPS 64
#define MAX_INSTALL_STACK 128

typedef struct {
    char path[512];
    int is_dir;
    char package_name[128];
    char main_file[128];
} PackageSource;

static void read_json_string(const char *json, const char *key, char *out, size_t out_size);
static const char *chpm_exe_dir = NULL;

static const char *get_home(void)
{
    const char *home = getenv("HOME");
    if (!home) { fprintf(stderr, "Error: HOME not set\n"); exit(1); }
    return home;
}

static const char *get_lib_dir(void)
{
    static char path[512];
    snprintf(path, sizeof(path), "%s/chpm_modules", get_home());
    return path;
}

static const char *get_packets_dir(void)
{
    static char path[512];
    snprintf(path, sizeof(path), "%s/packets_chpm_create", get_home());
    return path;
}

static void ensure_dir(const char *dir)
{
    char tmp[512];
    size_t len;

    if (!dir || dir[0] == '\0')
        return;

    snprintf(tmp, sizeof(tmp), "%s", dir);
    len = strlen(tmp);
    if (len > 1 && tmp[len - 1] == '/')
        tmp[len - 1] = '\0';

    for (char *p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST)
            {
                fprintf(stderr, "Error: cannot create directory %s\n", tmp);
                exit(1);
            }
            *p = '/';
        }
    }

    if (mkdir(tmp, 0755) != 0 && errno != EEXIST)
    {
        fprintf(stderr, "Error: cannot create directory %s\n", tmp);
        exit(1);
    }
}

static int valid_package_name(const char *name)
{
    int segment_len = 0;
    int slash_count = 0;
    int scoped = name && name[0] == '@';

    if (!name || name[0] == '\0' || name[0] == '/' || strstr(name, ".."))
        return 0;

    for (const char *p = name; *p; p++)
    {
        char c = *p;
        if (c == '/')
        {
            if (segment_len == 0)
                return 0;
            slash_count++;
            segment_len = 0;
            continue;
        }

        if (c == '@' && p != name)
            return 0;
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-' || c == '@'))
            return 0;
        segment_len++;
    }

    if (segment_len == 0)
        return 0;

    if (scoped)
    {
        const char *slash = strchr(name, '/');
        return slash_count == 1 && slash && slash > name + 1 && slash[1] != '\0';
    }

    return slash_count == 0;
}

static void trim_newline(char *value)
{
    size_t len = strlen(value);
    while (len > 0 && (value[len - 1] == '\n' || value[len - 1] == '\r'))
        value[--len] = '\0';
}

static void ensure_parent_dir(const char *path)
{
    char dir[512];
    char *slash;

    snprintf(dir, sizeof(dir), "%s", path);
    slash = strrchr(dir, '/');
    if (!slash)
        return;
    *slash = '\0';
    ensure_dir(dir);
}

static int file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static int dir_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static void copy_file(const char *src, const char *dst)
{
    FILE *fin = fopen(src, "rb");
    if (!fin) { fprintf(stderr, "Error: cannot read %s\n", src); exit(1); }
    FILE *fout = fopen(dst, "wb");
    if (!fout) { fclose(fin); fprintf(stderr, "Error: cannot write %s\n", dst); exit(1); }
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fin)) > 0)
        fwrite(buf, 1, n, fout);
    fclose(fin);
    fclose(fout);
}

static void copy_dir(const char *src, const char *dst)
{
    DIR *dir = opendir(src);
    if (!dir)
    {
        fprintf(stderr, "Error: cannot read directory %s\n", src);
        exit(1);
    }

    ensure_dir(dst);

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        char src_path[512], dst_path[512];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, ent->d_name);
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, ent->d_name);

        if (dir_exists(src_path))
            copy_dir(src_path, dst_path);
        else
        {
            ensure_parent_dir(dst_path);
            copy_file(src_path, dst_path);
        }
    }

    closedir(dir);
}

// --- package.json handling ---

static char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *content = malloc((size_t)size + 1);
    if (!content) { fclose(f); return NULL; }
    fread(content, 1, (size_t)size, f);
    content[size] = '\0';
    fclose(f);
    return content;
}

static void skip_ws(const char **p)
{
    while (**p == ' ' || **p == '\t' || **p == '\n' || **p == ',') (*p)++;
}

static void parse_deps(const char *content, char deps[][64], int *dep_count)
{
    *dep_count = 0;
    const char *p = strstr(content, "\"dependencies\"");
    if (!p) return;

    p += 14;
    while (*p && *p != '{') p++;
    if (*p != '{') return;
    p++;

    while (*p && *dep_count < MAX_DEPS)
    {
        skip_ws(&p);
        if (*p == '}' || *p == '\0') break;
        if (*p == '{') { p++; continue; }
        if (*p == '"')
        {
            p++;
            char name[64];
            int i = 0;
            while (*p && *p != '"' && i < 63) name[i++] = *p++;
            name[i] = '\0';
            if (*p == '"') p++;

            // skip colon
            while (*p && *p != ':') p++;
            if (*p == ':') p++;

            // skip value
            skip_ws(&p);
            if (*p == '"') { p++; while (*p && *p != '"') p++; if (*p == '"') p++; }
            else if (*p == '{') { p++; }
            else { while (*p && *p != ',' && *p != '}' && *p != '\n') p++; }

            if (name[0])
            {
                strcpy(deps[*dep_count], name);
                (*dep_count)++;
            }
        }
        else p++;
    }
}

static void add_dep_to_package_json(const char *path, const char *name)
{
    if (!file_exists(path))
    {
        FILE *f = fopen(path, "w");
        if (!f) return;
        fprintf(f, "{\n");
        fprintf(f, "  \"name\": \"my-project\",\n");
        fprintf(f, "  \"version\": \"1.0.0\",\n");
        fprintf(f, "  \"description\": \"\",\n");
        fprintf(f, "  \"license\": \"MIT\",\n");
        fprintf(f, "  \"main\": \"index.chp\",\n");
        fprintf(f, "  \"dependencies\": {\n");
        fprintf(f, "    \"%s\": \"0.1.0\"\n", name);
        fprintf(f, "  }\n");
        fprintf(f, "}\n");
        fclose(f);
        return;
    }

    char *content = read_file(path);
    if (!content) return;

    if (strstr(content, name)) { free(content); return; }

    // Find "dependencies" key
    char *key = strstr(content, "\"dependencies\"");
    if (!key) { free(content); return; }

    // Find opening { of deps object
    char *open_brace = key + 14;
    while (*open_brace && *open_brace != '{') open_brace++;
    if (*open_brace != '{') { free(content); return; }

    // Find matching closing }
    char *close_brace = open_brace + 1;
    int depth = 1;
    while (*close_brace && depth > 0)
    {
        if (*close_brace == '{') depth++;
        if (*close_brace == '}') depth--;
        if (depth > 0) close_brace++;
    }
    if (depth != 0) { free(content); return; }

    // Check if object already has items (look for " between { and })
    int has_items = 0;
    for (char *p = open_brace + 1; p < close_brace; p++)
        if (*p == '"') { has_items = 1; break; }

    char before[2048], after[2048], new_dep[128];

    if (!has_items)
    {
        // Empty deps: insert before }, adding newline + indent for } to follow
        long insert_pos = close_brace - content;
        strncpy(before, content, (size_t)insert_pos);
        before[insert_pos] = '\0';
        snprintf(new_dep, sizeof(new_dep), "\n    \"%s\": \"0.1.0\"\n  ", name);
        snprintf(after, sizeof(after), "%s", content + insert_pos);
    }
    else
    {
        // Has items: insert after last content, before trailing whitespace
        char *end_content = close_brace - 1;
        while (end_content > open_brace && (*end_content == ' ' || *end_content == '\t' || *end_content == '\n' || *end_content == '\r'))
            end_content--;
        long insert_pos = (end_content + 1) - content;
        strncpy(before, content, (size_t)insert_pos);
        before[insert_pos] = '\0';
        snprintf(new_dep, sizeof(new_dep), ",\n    \"%s\": \"0.1.0\"", name);
        snprintf(after, sizeof(after), "%s", content + insert_pos);
    }
    free(content);

    FILE *f = fopen(path, "w");
    if (f) { fprintf(f, "%s%s%s", before, new_dep, after); fclose(f); }
}

static int load_package_metadata(PackageSource *source, const char *fallback_name)
{
    char pkg_path[512];
    source->package_name[0] = '\0';
    source->main_file[0] = '\0';

    if (source->is_dir)
    {
        snprintf(pkg_path, sizeof(pkg_path), "%s/package.json", source->path);
        char *pkg = read_file(pkg_path);
        if (pkg)
        {
            read_json_string(pkg, "name", source->package_name, sizeof(source->package_name));
            read_json_string(pkg, "main", source->main_file, sizeof(source->main_file));
            free(pkg);
        }

        if (source->package_name[0] == '\0')
            snprintf(source->package_name, sizeof(source->package_name), "%s", fallback_name);
        if (source->main_file[0] == '\0')
            snprintf(source->main_file, sizeof(source->main_file), "main.chp");

        char main_path[512];
        snprintf(main_path, sizeof(main_path), "%s/%s", source->path, source->main_file);
        if (!file_exists(main_path))
        {
            snprintf(main_path, sizeof(main_path), "%s/main.chp", source->path);
            if (!file_exists(main_path))
                return 0;
            snprintf(source->main_file, sizeof(source->main_file), "main.chp");
        }
    }
    else
    {
        snprintf(source->package_name, sizeof(source->package_name), "%s", fallback_name);
        snprintf(source->main_file, sizeof(source->main_file), "%s.chp", fallback_name);
    }

    return 1;
}

static int try_package_dir(const char *path, const char *fallback_name, PackageSource *source)
{
    if (!dir_exists(path))
        return 0;

    snprintf(source->path, sizeof(source->path), "%s", path);
    source->is_dir = 1;
    return load_package_metadata(source, fallback_name);
}

static int try_package_file(const char *path, const char *fallback_name, PackageSource *source)
{
    if (!file_exists(path))
        return 0;

    snprintf(source->path, sizeof(source->path), "%s", path);
    source->is_dir = 0;
    return load_package_metadata(source, fallback_name);
}

static int try_scoped_package_dir(const char *base, const char *name, PackageSource *source)
{
    DIR *dir = opendir(base);
    if (!dir)
        return 0;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL)
    {
        if (ent->d_name[0] != '@')
            continue;

        char path[512], fallback[128];
        snprintf(path, sizeof(path), "%s/%s/%s", base, ent->d_name, name);
        snprintf(fallback, sizeof(fallback), "%s/%s", ent->d_name, name);
        if (try_package_dir(path, fallback, source))
        {
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;
}

static int resolve_package_source(const char *name, PackageSource *source)
{
    const char *packets = get_packets_dir();
    const char *bases[4];
    int base_count = 0;

    bases[base_count++] = "packets_chpm_create";
    bases[base_count++] = packets;
    if (chpm_exe_dir)
    {
        static char exe_packets[512];
        snprintf(exe_packets, sizeof(exe_packets), "%s/packets_chpm_create", chpm_exe_dir);
        bases[base_count++] = exe_packets;
    }
    bases[base_count++] = ".";

    for (int i = 0; i < base_count; i++)
    {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", bases[i], name);
        if (try_package_dir(path, name, source))
            return 1;

        snprintf(path, sizeof(path), "%s/%s.chp", bases[i], name);
        if (try_package_file(path, name, source))
            return 1;

        if (strchr(name, '/') == NULL && strchr(name, '@') == NULL)
        {
            if (try_scoped_package_dir(bases[i], name, source))
                return 1;
        }
    }

    return 0;
}

// --- Commands ---

static void cmd_init(void)
{
    if (file_exists("package.json"))
    {
        printf("package.json already exists\n");
        return;
    }

    char name[64] = "my-project";
    char version[16] = "1.0.0";
    char desc[256] = "";
    char author[128] = "";
    char license[64] = "MIT";
    char main_file[64] = "index.chp";

    printf("package name [my-project]: ");
    if (fgets(name, sizeof(name), stdin))
        name[strcspn(name, "\n")] = '\0';
    if (name[0] == '\0') snprintf(name, sizeof(name), "my-project");

    printf("version [1.0.0]: ");
    if (fgets(version, sizeof(version), stdin))
        version[strcspn(version, "\n")] = '\0';
    if (version[0] == '\0') snprintf(version, sizeof(version), "1.0.0");

    printf("description: ");
    if (fgets(desc, sizeof(desc), stdin))
        desc[strcspn(desc, "\n")] = '\0';

    printf("author: ");
    if (fgets(author, sizeof(author), stdin))
        author[strcspn(author, "\n")] = '\0';

    printf("license [MIT]: ");
    if (fgets(license, sizeof(license), stdin))
        license[strcspn(license, "\n")] = '\0';
    if (license[0] == '\0') snprintf(license, sizeof(license), "MIT");

    printf("entry point [index.chp]: ");
    if (fgets(main_file, sizeof(main_file), stdin))
        main_file[strcspn(main_file, "\n")] = '\0';
    if (main_file[0] == '\0') snprintf(main_file, sizeof(main_file), "index.chp");

    FILE *f = fopen("package.json", "w");
    fprintf(f, "{\n");
    fprintf(f, "  \"name\": \"%s\",\n", name);
    fprintf(f, "  \"version\": \"%s\",\n", version);
    fprintf(f, "  \"description\": \"%s\",\n", desc);
    if (author[0]) fprintf(f, "  \"author\": \"%s\",\n", author);
    fprintf(f, "  \"license\": \"%s\",\n", license);
    fprintf(f, "  \"main\": \"%s\",\n", main_file);
    fprintf(f, "  \"dependencies\": {}\n");
    fprintf(f, "}\n");
    fclose(f);

    if (!file_exists(main_file))
    {
        f = fopen(main_file, "w");
        if (f) { fprintf(f, "// %s v%s\nshow(\"Hello from %s!\")\n", name, version, name); fclose(f); }
    }

    printf("Created package '%s' v%s\n", name, version);
}

static void show_progress(const char *label)
{
    printf("  %s ", label);
    fflush(stdout);
    const char bar[] = "|/-\\";
    for (int i = 0; i < 10; i++)
    {
        printf("\b%c", bar[i % 4]);
        fflush(stdout);
        for (volatile int j = 0; j < 5000000; j++);
    }
    printf("\b");
}

static void chpm_set_exe_dir(const char *argv0)
{
    static char dir[512];
    const char *slash = strrchr(argv0, '/');
    if (slash)
    {
        size_t len = (size_t)(slash - argv0);
        strncpy(dir, argv0, len);
        dir[len] = '\0';
        chpm_exe_dir = dir;
    }
}

static int install_seen(const char *name, char seen[][128], int seen_count)
{
    for (int i = 0; i < seen_count; i++)
        if (strcmp(seen[i], name) == 0)
            return 1;
    return 0;
}

static void install_package(const char *name, int save_dependency, char seen[][128], int *seen_count)
{
    if (!valid_package_name(name))
    {
        fprintf(stderr, "Invalid package name: %s\n", name);
        return;
    }

    if (install_seen(name, seen, *seen_count))
        return;
    if (*seen_count < MAX_INSTALL_STACK)
    {
        snprintf(seen[*seen_count], 128, "%s", name);
        (*seen_count)++;
    }

    ensure_dir("chpm_modules");
    PackageSource source;
    if (!resolve_package_source(name, &source))
    {
        printf("'%s' not found in packets_chpm_create or current dir\n", name);
        return;
    }

    char pkg_json[512];
    if (source.is_dir)
    {
        snprintf(pkg_json, sizeof(pkg_json), "%s/package.json", source.path);
        char *content = read_file(pkg_json);
        if (content)
        {
            char deps[MAX_DEPS][64];
            int dep_count = 0;
            parse_deps(content, deps, &dep_count);
            free(content);
            for (int i = 0; i < dep_count; i++)
                install_package(deps[i], 0, seen, seen_count);
        }
    }

    show_progress("installing");
    char dst[512];
    if (source.is_dir)
    {
        snprintf(dst, sizeof(dst), "chpm_modules/%s", source.package_name);
        copy_dir(source.path, dst);
    }
    else
    {
        if (source.package_name[0] == '@')
            snprintf(dst, sizeof(dst), "chpm_modules/%s/main.chp", source.package_name);
        else
            snprintf(dst, sizeof(dst), "chpm_modules/%s.chp", source.package_name);
        ensure_parent_dir(dst);
        copy_file(source.path, dst);
    }
    printf(" OK\n");

    if (save_dependency && file_exists("package.json"))
        add_dep_to_package_json("package.json", source.package_name);
}

static void cmd_install_single(const char *name)
{
    char seen[MAX_INSTALL_STACK][128];
    int seen_count = 0;
    install_package(name, 1, seen, &seen_count);
}

static void cmd_install_all(void)
{
    char *content = read_file("package.json");
    if (!content)
    {
        printf("No package.json found\n");
        return;
    }

    char deps[MAX_DEPS][64];
    int dep_count = 0;
    parse_deps(content, deps, &dep_count);
    free(content);

    if (dep_count == 0)
    {
        printf("No dependencies found in package.json\n");
        return;
    }

    printf("Installing %d dependencies...\n\n", dep_count);
    for (int i = 0; i < dep_count; i++)
    {
        printf("  ");
        cmd_install_single(deps[i]);
    }
}

static void cmd_remove(const char *name)
{
    char path[512];
    snprintf(path, sizeof(path), "chpm_modules/%s.chp", name);
    if (remove(path) == 0)
        printf("Removed %s\n", name);
    else
        fprintf(stderr, "'%s' not installed in ./chpm_modules/\n", name);
}

static void read_json_string(const char *json, const char *key, char *out, size_t out_size)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *start = strstr(json, search);
    if (!start) { out[0] = '\0'; return; }
    start = strchr(start + strlen(key) + 2, '"');
    if (!start) { out[0] = '\0'; return; }
    start++;
    const char *end = strchr(start, '"');
    if (!end) { out[0] = '\0'; return; }
    size_t len = (size_t)(end - start);
    if (len >= out_size) len = out_size - 1;
    strncpy(out, start, len);
    out[len] = '\0';
}

static void cmd_list(void)
{
    char pkg_name[64] = "?", pkg_ver[64] = "?";
    if (file_exists("package.json"))
    {
        char *pkg = read_file("package.json");
        if (pkg)
        {
            read_json_string(pkg, "name", pkg_name, sizeof(pkg_name));
            read_json_string(pkg, "version", pkg_ver, sizeof(pkg_ver));
            free(pkg);
        }
    }

    printf("Project: %s v%s\n\n", pkg_name, pkg_ver);

    if (!file_exists("chpm_modules"))
    {
        printf("  No modules installed\n");
        return;
    }

    printf("%-20s %10s\n", "Module", "Size");
    printf("%-20s %10s\n", "------", "----");

    // Read deps from package.json
    int dep_count = 0;
    char deps[64][64];
    if (file_exists("package.json"))
    {
        char *pkg = read_file("package.json");
        if (pkg)
        {
            char *in_deps = strstr(pkg, "\"dependencies\"");
            if (in_deps)
            {
                char *p = strchr(in_deps, '{');
                if (p)
                {
                    p++;
                    char *end = strchr(p, '}');
                    if (end) *end = '\0';
                    char *tok = strtok(p, "\",");
                    while (tok && dep_count < 64)
                    {
                        while (*tok == ' ' || *tok == '\n' || *tok == '\t' || *tok == '\r') tok++;
                        if (*tok && *tok != ':')
                        {
                            strncpy(deps[dep_count], tok, 63);
                            deps[dep_count][63] = '\0';
                            dep_count++;
                        }
                        tok = strtok(NULL, "\",");
                    }
                }
            }
            free(pkg);
        }
    }

    int found = 0;
    for (int i = 0; i < dep_count; i++)
    {
        char path[512];
        snprintf(path, sizeof(path), "chpm_modules/%s.chp", deps[i]);
        struct stat st;
        if (stat(path, &st) != 0)
            snprintf(path, sizeof(path), "chpm_modules/%s", deps[i]);
        if (stat(path, &st) == 0)
        {
            printf("%-20s %10ld\n", deps[i], (long)st.st_size);
            found = 1;
        }
    }

    if (!found)
        printf("  (none)\n");
}

static void cmd_info(const char *name)
{
    char path[512];
    struct stat st;

    // Check local ./chpm_modules/ first, then global ~/chpm_modules/
    snprintf(path, sizeof(path), "chpm_modules/%s.chp", name);
    if (stat(path, &st) != 0)
    {
        snprintf(path, sizeof(path), "chpm_modules/%s/main.chp", name);
        if (stat(path, &st) != 0)
        {
            const char *lib = get_lib_dir();
            snprintf(path, sizeof(path), "%s/%s.chp", lib, name);
            if (stat(path, &st) != 0)
            {
                snprintf(path, sizeof(path), "%s/%s/main.chp", lib, name);
                if (stat(path, &st) != 0)
                {
                    printf("'%s' is not installed\n", name);
                    return;
                }
            }
        }
    }

    char *content = read_file(path);
    int lines = 1, funcs = 0, vars = 0;
    if (content)
    {
        for (char *p = content; *p; p++)
        {
            if (*p == '\n') lines++;
            if (strncmp(p, "func ", 5) == 0) funcs++;
            if (strstr(p, "= "))
            {
                // check it's an assignment (not in func body)
                char *before = p;
                while (before > content && *before != '\n') before--;
                if (strncmp(before, "func ", 5) != 0)
                    vars++;
                p = strstr(p, "= ");
            }
        }
        free(content);
    }

    printf("Module:  %s\n", name);
    printf("Path:    %s\n", path);
    printf("Size:    %ld bytes\n", (long)st.st_size);
    printf("Lines:   %d\n", lines);
    printf("Values:  %d\n", vars);
    printf("Funcs:   %d\n", funcs);
    printf("Updated: %s", ctime(&st.st_mtime));
}

static void cmd_create(const char *name)
{
    if (!name || name[0] == '\0')
    {
        fprintf(stderr, "Usage: chpm create <name>\n");
        return;
    }

    char package_name[256];
    char author[128] = "";

    if (name[0] == '@')
    {
        snprintf(package_name, sizeof(package_name), "%s", name);
        const char *slash = strchr(name, '/');
        if (slash && slash > name + 1)
        {
            size_t author_len = (size_t)(slash - name - 1);
            if (author_len >= sizeof(author))
                author_len = sizeof(author) - 1;
            strncpy(author, name + 1, author_len);
            author[author_len] = '\0';
        }
    }
    else
    {
        printf("author username: @");
        if (!fgets(author, sizeof(author), stdin))
        {
            fprintf(stderr, "Author is required\n");
            return;
        }
        trim_newline(author);
        if (author[0] == '@')
            memmove(author, author + 1, strlen(author));
        if (author[0] == '\0')
        {
            fprintf(stderr, "Author is required\n");
            return;
        }
        snprintf(package_name, sizeof(package_name), "@%s/%s", author, name);
    }

    if (!valid_package_name(package_name))
    {
        fprintf(stderr, "Invalid package name: %s\n", package_name);
        fprintf(stderr, "Use 'module-name' or '@creator/module-name'\n");
        return;
    }

    if (file_exists(package_name))
    {
        fprintf(stderr, "Directory '%s' already exists\n", package_name);
        return;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/lib", package_name);
    ensure_dir(filepath);

    snprintf(filepath, sizeof(filepath), "%s/package.json", package_name);
    FILE *f = fopen(filepath, "w");
    fprintf(f, "{\n");
    fprintf(f, "  \"name\": \"%s\",\n", package_name);
    fprintf(f, "  \"version\": \"1.0.0\",\n");
    fprintf(f, "  \"description\": \"\",\n");
    fprintf(f, "  \"author\": \"%s\",\n", author);
    fprintf(f, "  \"license\": \"MIT\",\n");
    fprintf(f, "  \"main\": \"main.chp\",\n");
    fprintf(f, "  \"dependencies\": {}\n");
    fprintf(f, "}\n");
    fclose(f);

    const char *import_name = strrchr(package_name, '/');
    import_name = import_name ? import_name + 1 : package_name;

    snprintf(filepath, sizeof(filepath), "%s/main.chp", package_name);
    f = fopen(filepath, "w");
    fprintf(f, "// %s v1.0.0\n", package_name);
    fprintf(f, "export default package {\n");
    fprintf(f, "    func hello(name) {\n");
    fprintf(f, "        return \"Hello \" + name + \" from %s\"\n", package_name);
    fprintf(f, "    }\n");
    fprintf(f, "}\n");
    fclose(f);

    snprintf(filepath, sizeof(filepath), "%s/README.md", package_name);
    f = fopen(filepath, "w");
    fprintf(f, "# %s\n\n", package_name);
    fprintf(f, "This package is a standalone chpm project. Users install it first, then import it by the module name.\n\n");
    fprintf(f, "```chipojo\n");
    fprintf(f, "import pkg from \"%s\"\n", import_name);
    fprintf(f, "show(pkg.hello(\"Chipojo\"))\n");
    fprintf(f, "```\n");
    fclose(f);

    snprintf(filepath, sizeof(filepath), "%s/PUBLISHING.md", package_name);
    f = fopen(filepath, "w");
    fprintf(f, "# Publishing\n\n");
    fprintf(f, "Users do not publish directly to the chpm repository or database.\n\n");
    fprintf(f, "Send this package folder name to a chpm admin:\n\n");
    fprintf(f, "```text\n%s\n```\n\n", package_name);
    fprintf(f, "The admin reviews the project and uploads it to the official chpm repository/database.\n");
    fclose(f);

    printf("Created package '%s' with:\n", package_name);
    printf("  %s/package.json\n", package_name);
    printf("  %s/main.chp\n", package_name);
    printf("  %s/lib/\n", package_name);
    printf("  %s/README.md\n", package_name);
    printf("  %s/PUBLISHING.md\n", package_name);
}

void print_usage(void)
{
    printf(CHPM_MADE " v" CHPM_VERSION "\n");
    printf("by " CHPM_AUTHOR "\n\n");
    printf("Usage: chpm <command> [options]\n\n");
    printf("Project commands:\n");
    printf("  init                   Create package.json\n");
    printf("  create <name>          Create @author/name package scaffold\n");
    printf("  create @creator/name   Create scoped package project\n\n");
    printf("Package commands:\n");
    printf("  install, i             Install all deps from package.json\n");
    printf("  install, i <name>      Install module to ./chpm_modules/\n");
    printf("  install, i @creator/name\n");
    printf("                         Install scoped module to ./chpm_modules/@creator/name/\n");
    printf("  remove <name>          Remove module from ./chpm_modules/\n\n");
    printf("Info commands:\n");
    printf("  list                   List installed modules\n");
    printf("  info <name>            Show module details\n");
    printf("  -v, --version          Show version\n");
}

int main(int argc, char **argv)
{
    chpm_set_exe_dir(argv[0]);

    if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0))
    {
        print_version();
        return 0;
    }

    if (argc < 2)
    {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "init") == 0)
        cmd_init();
    else if (strcmp(argv[1], "create") == 0)
        cmd_create(argc >= 3 ? argv[2] : NULL);
    else if (strcmp(argv[1], "install") == 0 || strcmp(argv[1], "i") == 0)
    {
        if (argc >= 3) cmd_install_single(argv[2]);
        else cmd_install_all();
    }
    else if (strcmp(argv[1], "remove") == 0)
    {
        if (argc < 3) { fprintf(stderr, "Usage: chpm remove <name>\n"); return 1; }
        cmd_remove(argv[2]);
    }
    else if (strcmp(argv[1], "list") == 0)
        cmd_list();
    else if (strcmp(argv[1], "info") == 0)
    {
        if (argc < 3) { fprintf(stderr, "Usage: chpm info <name>\n"); return 1; }
        cmd_info(argv[2]);
    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        fprintf(stderr, "Try 'chpm' for usage\n");
        return 1;
    }

    return 0;
}
