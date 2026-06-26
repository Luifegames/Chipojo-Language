#include "variables.h"
#include "chpio.h"
#include "lexer.h"
#include "parser.h"
#include <ctype.h>
#include <io.h>
#include <dirent.h>

#define MAX_VARS 100
#define MAX_MODULES 64
#define MAX_MODULE_BUFFERS 64

Scope scope_stack[MAX_SCOPE];
int scope_depth = 0;

// Keep module input buffers alive so function body start indices remain valid
static char *module_buffers[MAX_MODULE_BUFFERS];
static int module_buffer_count = 0;

// Module cache
typedef struct {
    char *name;
    Value module;
} ModuleEntry;

static ModuleEntry module_cache[MAX_MODULES];
static int module_count = 0;

static Dict *clone_dict(Dict *src);
static List *clone_list(List *src);

extern char *input;
extern int indx;
extern int g_line;
extern Token current_token;

void save_interpreter_state(char **saved_input, int *saved_indx, int *saved_line, Token *saved_token, int *saved_scope)
{
    *saved_input = input;
    *saved_indx = indx;
    *saved_line = g_line;
    *saved_token = current_token;
    *saved_scope = scope_depth;
}

void restore_interpreter_state(char *saved_input, int saved_indx, int saved_line, Token saved_token, int saved_scope)
{
    input = saved_input;
    indx = saved_indx;
    g_line = saved_line;
    current_token = saved_token;
    // Pop any scopes that were pushed during module loading
    while (scope_depth > saved_scope)
        pop_scope();
}

static int module_is_loaded(char *name)
{
    for (int i = 0; i < module_count; i++)
        if (strcmp(module_cache[i].name, name) == 0)
            return 1;
    return 0;
}

static void normalize_module_name(const char *name, char *out, size_t out_size)
{
    size_t j = 0;
    for (size_t i = 0; name[i] && j + 1 < out_size; i++)
    {
        char c = name[i];
        out[j++] = c;
    }
    out[j] = '\0';
}

static int try_package_json_main(const char *dir, char *out, int out_size);

static int try_scoped_module_path(const char *base, const char *name, char *out, int out_size)
{
    DIR *dir = opendir(base);
    if (!dir)
        return 0;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL)
    {
        if (ent->d_name[0] != '@')
            continue;

        char tmp[512];
        snprintf(tmp, sizeof(tmp), "%s/%s/%s.chp", base, ent->d_name, name);
        FILE *f = fopen(tmp, "rb");
        if (f)
        {
            fclose(f);
            strncpy(out, tmp, (size_t)out_size - 1);
            out[out_size - 1] = '\0';
            closedir(dir);
            return 1;
        }

        snprintf(tmp, sizeof(tmp), "%s/%s/%s", base, ent->d_name, name);
        if (try_package_json_main(tmp, out, out_size))
        {
            closedir(dir);
            return 1;
        }

        snprintf(tmp, sizeof(tmp), "%s/%s/%s/main.chp", base, ent->d_name, name);
        f = fopen(tmp, "rb");
        if (f)
        {
            fclose(f);
            strncpy(out, tmp, (size_t)out_size - 1);
            out[out_size - 1] = '\0';
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;
}

static char *read_text_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *content = malloc((size_t)size + 1);
    if (!content)
    {
        fclose(f);
        return NULL;
    }
    fread(content, 1, (size_t)size, f);
    content[size] = '\0';
    fclose(f);
    return content;
}

static void read_json_string_field(const char *json, const char *key, char *out, size_t out_size)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *start = strstr(json, search);
    if (!start) { out[0] = '\0'; return; }
    start = strchr(start + strlen(search), ':');
    if (!start) { out[0] = '\0'; return; }
    start++;
    while (*start && isspace((unsigned char)*start)) start++;
    if (*start != '"') { out[0] = '\0'; return; }
    start++;
    const char *end = strchr(start, '"');
    if (!end) { out[0] = '\0'; return; }
    size_t len = (size_t)(end - start);
    if (len >= out_size) len = out_size - 1;
    strncpy(out, start, len);
    out[len] = '\0';
}

static int try_package_json_main(const char *dir, char *out, int out_size)
{
    char pkg_path[512], main_file[256], tmp[512];
    snprintf(pkg_path, sizeof(pkg_path), "%s/package.json", dir);
    char *pkg = read_text_file(pkg_path);
    if (!pkg)
        return 0;

    read_json_string_field(pkg, "main", main_file, sizeof(main_file));
    free(pkg);

    if (main_file[0] == '\0')
        snprintf(main_file, sizeof(main_file), "main.chp");

    snprintf(tmp, sizeof(tmp), "%s/%s", dir, main_file);
    FILE *f = fopen(tmp, "rb");
    if (f)
    {
        fclose(f);
        strncpy(out, tmp, (size_t)out_size - 1);
        out[out_size - 1] = '\0';
        return 1;
    }

    return 0;
}

static Value *get_cached_module(char *name)
{
    for (int i = 0; i < module_count; i++)
        if (strcmp(module_cache[i].name, name) == 0)
            return &module_cache[i].module;
    return NULL;
}

static Dict *clone_dict(Dict *src)
{
    if (!src) return NULL;
    Dict *d = malloc(sizeof(Dict));
    d->count = src->count;
    d->capacity = src->capacity;
    d->entries = malloc((size_t)d->capacity * sizeof(DictEntry));
    for (int i = 0; i < d->count; i++)
    {
        d->entries[i].key = strdup(src->entries[i].key);
        d->entries[i].value = malloc(sizeof(Value));
        *(d->entries[i].value) = clone_value(*(src->entries[i].value));
    }
    return d;
}

static List *clone_list(List *src)
{
    if (!src) return NULL;
    List *list = malloc(sizeof(List));
    list->count = src->count;
    list->capacity = src->capacity;
    list->items = malloc((size_t)list->capacity * sizeof(Value));
    for (int i = 0; i < list->count; i++)
        list->items[i] = clone_value(src->items[i]);
    return list;
}

static void cache_module(char *name, Value mod_val)
{
    if (module_count < MAX_MODULES)
    {
        module_cache[module_count].name = strdup(name);
        // Deep-copy dict for cache so it survives caller freeing their copy
        Value cache_val = mod_val;
        if (mod_val.type == VAR_MODULE && mod_val.value.dict)
            cache_val.value.dict = clone_dict(mod_val.value.dict);
        module_cache[module_count].module = cache_val;
        module_count++;
    }
}

void free_value_internals(Value *val)
{
    if (!val) return;
    if (val->type == VAR_STRING)
    {
        free(val->value.str);
        val->value.str = NULL;
    }
    else if (val->type == VAR_LIST)
    {
        List *list = val->value.list;
        if (list)
        {
            for (int j = 0; j < list->count; j++)
            {
                free_value_internals(&list->items[j]);
            }
            free(list->items);
            free(list);
        }
        val->value.list = NULL;
    }
    else if (val->type == VAR_DICT)
    {
        Dict *dict = val->value.dict;
        if (dict)
        {
            for (int j = 0; j < dict->count; j++)
            {
                free(dict->entries[j].key);
                if (dict->entries[j].value)
                {
                    free_value_internals(dict->entries[j].value);
                    free(dict->entries[j].value);
                }
            }
            free(dict->entries);
            free(dict);
        }
        val->value.dict = NULL;
    }
    else if (val->type == VAR_FUNCTION)
    {
        for (int j = 0; j < val->value.func.param_count; j++)
        {
            free(val->value.func.param[j]);
        }
        free(val->value.func.param);
        if (val->value.func.closure)
        {
            Dict *d = val->value.func.closure;
            for (int j = 0; j < d->count; j++)
            {
                free(d->entries[j].key);
                if (d->entries[j].value)
                {
                    free_value_internals(d->entries[j].value);
                    free(d->entries[j].value);
                }
            }
            free(d->entries);
            free(d);
        }
        // buffer is a borrowed pointer to module input, never freed here
        val->value.func.param = NULL;
        val->value.func.param_count = 0;
        val->value.func.closure = NULL;
    }
    else if (val->type == VAR_MODULE)
    {
        Dict *dict = val->value.dict;
        if (dict)
        {
            for (int j = 0; j < dict->count; j++)
            {
                free(dict->entries[j].key);
                if (dict->entries[j].value)
                {
                    free_value_internals(dict->entries[j].value);
                    free(dict->entries[j].value);
                }
            }
            free(dict->entries);
            free(dict);
        }
        val->value.dict = NULL;
    }
}

Value clone_value(Value v)
{
    Value copy = {0};

    copy.type = v.type;
    copy.exported = v.exported;
    copy.is_const = v.is_const;

    switch (v.type)
    {
    case VAR_NUMBER:
        copy.value.num = v.value.num;
        break;

    case VAR_STRING:
        copy.value.str = strdup(v.value.str);
        break;

    case VAR_DICT:
        copy.value.dict = clone_dict(v.value.dict);
        break;

    case VAR_FUNCTION:
        copy.value.func.start = v.value.func.start;
        copy.value.func.param_count = v.value.func.param_count;
        copy.value.func.param = malloc((size_t)v.value.func.param_count * sizeof(char *));
        for (int i = 0; i < v.value.func.param_count; i++)
            copy.value.func.param[i] = strdup(v.value.func.param[i]);
        // buffer is a borrowed pointer to module input, never owned
        copy.value.func.buffer = v.value.func.buffer;
        copy.value.func.is_block = v.value.func.is_block;
        copy.value.func.closure = v.value.func.closure ? clone_dict(v.value.func.closure) : NULL;
        break;

    case VAR_NATIVE:
        copy.value.native_func = v.value.native_func;
        break;
    case VAR_LIST:
        copy.value.list = clone_list(v.value.list);
        break;

    case VAR_NULL:
        break;

    case VAR_MODULE:
        copy.value.dict = clone_dict(v.value.dict);
        break;

    default:
        runtime_error("Error in clone value method");
        break;
    }

    return copy;
}

void list_push(List *list, Value val)
{
    if (list->count >= list->capacity)
    {
        list->capacity *= 2;
        list->items = realloc(list->items, sizeof(Value) * (size_t)list->capacity);
    }

    list->items[list->count++] = clone_value(val);
}

void dict_new(char *name, Dict *dict){

    Scope *sc = &scope_stack[scope_depth];
    for (int d = 0; d <= scope_depth; d++)
    {
        sc = &scope_stack[d];
        for (int i = 0; i < sc->count; i++)
        {
            if (strcmp(sc->vars[i]->name, name) == 0)
            {
                free_value_internals(sc->vars[i]);
                sc->vars[i]->type = VAR_DICT;
                sc->vars[i]->value.dict = dict;

                return;
            }
        }
    }
    sc = &scope_stack[scope_depth];
    Value *new_var = malloc(sizeof(Value));
    new_var->name = strdup(name);
    new_var->type = VAR_DICT;
    new_var->exported = 0;
    new_var->value.dict = dict;
    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;
    new_var->value.func.buffer = NULL;

    sc->vars[sc->count++] = new_var;
}

void list_new(char *name, List *list)
{

    Scope *sc = &scope_stack[scope_depth];
    for (int d = 0; d <= scope_depth; d++)
    {
        sc = &scope_stack[d];
        for (int i = 0; i < sc->count; i++)
        {
            if (strcmp(sc->vars[i]->name, name) == 0)
            {
                free_value_internals(sc->vars[i]);
                sc->vars[i]->type = VAR_LIST;
                sc->vars[i]->value.list = list;
                return;
            }
        }
    }

    sc = &scope_stack[scope_depth];
    Value *new_var = malloc(sizeof(Value));

    new_var->name = strdup(name);
    new_var->type = VAR_LIST;
    new_var->exported = 0;
    new_var->value.list = list;
    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;
    new_var->value.func.buffer = NULL;

    sc->vars[sc->count++] = new_var;
}

void dict_set(Dict *dict, char *key, Value *val){
    
    for (int i = 0; i < dict->count; i++)
    {
        if (strcmp(dict ->entries[i].key, key) == 0)
        {
            free_value_internals(dict->entries[i].value);
            *(dict->entries[i].value) = clone_value(*val);

            return;
        }
    }
    
    if (dict->count >= dict->capacity)
    {
        dict->capacity *= 2;
        dict->entries = realloc(dict->entries, (size_t)dict->capacity * sizeof(DictEntry));
    }
    dict->entries[dict->count].key = strdup(key);
    dict->entries[dict->count].value = malloc(sizeof(Value));
    *(dict->entries[dict->count++].value) = clone_value(*val);
}

Value dict_get(Dict *d, char *key)
{
    for (int i = 0; i < d->count; i++)
    {
        if (strcmp(d->entries[i].key, key) == 0)
        {
            return clone_value(*d->entries[i].value);
        }
    }
    Value null_val;
    null_val.type = VAR_NULL;
    return null_val;
}

void assign_number_val(char *name, double num)
{
    Scope *sc = &scope_stack[scope_depth];
    for (int d = 0; d <= scope_depth; d++)
    {
        sc = &scope_stack[d];
        for (int i = 0; i < sc->count; i++)
        {
            if (strcmp(sc->vars[i]->name, name) == 0)
            {
                free_value_internals(sc->vars[i]);
                sc->vars[i]->type = VAR_NUMBER;
                sc->vars[i]->value.num = num;

                return;
            }
        }
    }
    sc = &scope_stack[scope_depth];
    Value *new_var = malloc(sizeof(Value));

    new_var->name = strdup(name);
    new_var->type = VAR_NUMBER;
    new_var->exported = 0;
    new_var->is_const = 0;
    new_var->value.num = num;

    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;
    new_var->value.func.buffer = NULL;

    sc->vars[sc->count++] = new_var;
}

void assign_string_val(char *name, char *str_value)
{
    Scope *sc = &scope_stack[scope_depth];
    for (int d = 0; d <= scope_depth; d++)
    {
        sc = &scope_stack[d];
        for (int i = 0; i < sc->count; i++)
        {
            if (strcmp(sc->vars[i]->name, name) == 0)
            {
                free_value_internals(sc->vars[i]);
                sc->vars[i]->type = VAR_STRING;
                sc->vars[i]->value.str = strdup(str_value);
                return;
            }
        }
    }

    sc = &scope_stack[scope_depth];
    Value *new_var = malloc(sizeof(Value));

    new_var->name = strdup(name);
    new_var->type = VAR_STRING;
    new_var->exported = 0;
    new_var->is_const = 0;
    new_var->value.str = strdup(str_value);

    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;

    sc->vars[sc->count++] = new_var;

}

void define_function(char *name, int start, char **params, int param_count, int exported, char *buffer)
{
    Scope *sc = &scope_stack[scope_depth];
    for (int i = 0; i < sc->count; i++)
        if (strcmp(sc->vars[i]->name, name) == 0)
        {
            char error[256];
            snprintf(error, sizeof(error), "Function %s is defined twice", name);
            syntax_error_line(error, current_token.line);
        }

    Value *new_var = malloc(sizeof(Value));
    new_var->name = strdup(name);
    new_var->type = VAR_FUNCTION;
    new_var->exported = exported;
    new_var->is_const = 0;
    new_var->value.func.start = start;

    new_var->value.func.param = malloc((size_t)param_count * sizeof(char *));
    for (int i = 0; i < param_count; i++)
        new_var->value.func.param[i] = strdup(params[i]);

    new_var->value.func.param_count = param_count;
    new_var->value.func.buffer = buffer;
    new_var->value.func.is_block = 1;
    sc->vars[sc->count++] = new_var;

}

void assign_null_val(char *name)
{

    Scope *sc = &scope_stack[scope_depth];
    for (int d = 0; d <= scope_depth; d++)
    {
        sc = &scope_stack[d];
        for (int i = 0; i < sc->count; i++)
        {
            if (strcmp(sc->vars[i]->name, name) == 0)
            {
                free_value_internals(sc->vars[i]);
                sc->vars[i]->type = VAR_NULL;
                return;
            }
        }
    }
    sc = &scope_stack[scope_depth];
    Value *new_var = malloc(sizeof(Value));
    new_var->name = strdup(name);
    new_var->type = VAR_NULL;
    new_var->exported = 0;
    sc->vars[sc->count++] = new_var;
    }

Value var_value_get(char *name)
{
    Value v = {0};
    for (int d = scope_depth; d >= 0; d--)
    {
        Scope *sc = &scope_stack[d];
        for (int i = 0; i < sc->count; i++)
            if (strcmp(sc->vars[i]->name, name) == 0)
            {
                v.name = sc->vars[i]->name;

                v.type = sc->vars[i]->type;
                v.exported = sc->vars[i]->exported;
                v.is_const = sc->vars[i]->is_const;
                if (v.type == VAR_NUMBER)
                    v.value.num = sc->vars[i]->value.num;
                else if (v.type == VAR_STRING)
                    v.value.str = strdup(sc->vars[i]->value.str);
                else if (v.type == VAR_FUNCTION)
                {
                    v.value.func.start = sc->vars[i]->value.func.start;
                    v.value.func.param = sc->vars[i]->value.func.param;
                    v.value.func.param_count = sc->vars[i]->value.func.param_count;
                    v.value.func.buffer = sc->vars[i]->value.func.buffer;
                    v.value.func.is_block = sc->vars[i]->value.func.is_block;
                }
                else if (v.type == VAR_DICT)
                {
                    v.value.dict = sc->vars[i]->value.dict;
                }
                else if (v.type == VAR_LIST)
                {
                    v.value.list = sc->vars[i]->value.list;
                }
                else if (v.type == VAR_NULL)
                {
                    v.name = "null";
                    v.type = VAR_NULL;
                }
                else if (v.type == VAR_NATIVE)
                {
                    v.value.native_func = sc->vars[i]->value.native_func;
                }
                else if (v.type == VAR_MODULE)
                {
                    v.value.dict = sc->vars[i]->value.dict;
                }
                else{
                    char message[64];
                    printf("%s\n",v.name);
                    sprintf(message, "Can't get this variable, var type %d", v.type);
                    runtime_error(message);
                }
                return v;
            }
    }
    undefined_variable_error(name, current_token.line);
    return v;
}

void variable_set(char *name, Value value){
    
    if (value.type == VAR_STRING)
    {
        assign_string_val(name, value.value.str);
        }
    else if (value.type == VAR_NULL)
    {
        assign_null_val(name);
    }
    else if (value.type == VAR_NUMBER)
    {
        assign_number_val(name, value.value.num);
    }
    else if (value.type == VAR_DICT)
    {
        Value copy = clone_value(value);
        dict_new(name, copy.value.dict);
    }
    else if (value.type == VAR_LIST)
    {
        Value copy = clone_value(value);
        list_new(name, copy.value.list);
    }
    else if (value.type == VAR_NATIVE)
    {
        Scope *sc = &scope_stack[scope_depth];
        Value *new_var = malloc(sizeof(Value));

        new_var->name = strdup(name);
        new_var->exported = 0;
        new_var->is_const = 0;
        new_var->type = VAR_NATIVE;
        new_var->value = value.value;
        sc->vars[sc->count++] = new_var;
    }
    else if (value.type == VAR_MODULE)
    {
        Scope *sc = &scope_stack[scope_depth];
        Value *new_var = malloc(sizeof(Value));
        new_var->name = strdup(name);
        new_var->exported = 0;
        new_var->is_const = 0;
        new_var->type = VAR_MODULE;
        new_var->value.dict = clone_dict(value.value.dict);
        sc->vars[sc->count++] = new_var;
    }
    else if (value.type == VAR_FUNCTION)
    {
        Scope *sc = &scope_stack[scope_depth];
        Value *new_var = malloc(sizeof(Value));
        new_var->name = strdup(name);
        new_var->exported = 0;
        new_var->is_const = 0;
        new_var->is_const = 0;
        new_var->type = VAR_FUNCTION;
        new_var->value.func.start = value.value.func.start;
        new_var->value.func.param_count = value.value.func.param_count;
        new_var->value.func.param = malloc((size_t)value.value.func.param_count * sizeof(char *));
        for (int i = 0; i < value.value.func.param_count; i++)
            new_var->value.func.param[i] = strdup(value.value.func.param[i]);
        new_var->value.func.buffer = value.value.func.buffer;
        new_var->value.func.is_block = value.value.func.is_block;
        sc->vars[sc->count++] = new_var;
    }
    else{
        undefined_variable_error(name, current_token.line);
    }
}





Value list_get(List *list, int index){

    if (index < 0 || index >= list->count){
        runtime_error("List index out of range");
    }

    return clone_value(list->items[index]);
}

void push_scope(){
    if (scope_depth + 1 >= MAX_SCOPE)
    {
        runtime_error("Maximum scope depth exceeded");
    }
    scope_stack[++scope_depth].count = 0;
}

void pop_scope()
{
    if (scope_depth > 0)
    {
        Scope *sc = &scope_stack[scope_depth];
        for (int i = 0; i < sc->count; i++)
        {
            free(sc->vars[i]->name);
            free_value_internals(sc->vars[i]);
            free(sc->vars[i]);
        }
        scope_depth--;
    }
}

// Try to open a module file, checking .chp first then directory/main.chp
static int try_module_path(const char *base, const char *name, char *out, int out_size)
{
    char tmp[512];
    // Try <base>/<name>.chp
    snprintf(tmp, sizeof(tmp), "%s/%s.chp", base, name);
    FILE *f = fopen(tmp, "rb");
    if (f) { fclose(f); strncpy(out, tmp, (size_t)out_size - 1); out[out_size - 1] = '\0'; return 1; }
    // Try <base>/<name>/<package.json main> or <base>/<name>/main.chp (directory package)
    snprintf(tmp, sizeof(tmp), "%s/%s", base, name);
    if (try_package_json_main(tmp, out, out_size))
        return 1;

    snprintf(tmp, sizeof(tmp), "%s/%s/main.chp", base, name);
    f = fopen(tmp, "rb");
    if (f) { fclose(f); strncpy(out, tmp, (size_t)out_size - 1); out[out_size - 1] = '\0'; return 1; }
    return 0;
}

static int default_block_starts_here()
{
    if (current_token.type != TOKEN_ID)
        return 0;

    int i = indx;
    while (input[i] && isspace((unsigned char)input[i]))
        i++;
    return input[i] == '{';
}

static Value parse_default_block()
{
    char block_name[64];
    strcpy(block_name, current_token.name);
    forward();

    push_scope();
    consume(TOKEN_LEFTBRACE, "expected '{'");
    while (current_token.type != TOKEN_RIGHTBRACE)
    {
        if (current_token.type == TOKEN_FUNC || current_token.type == TOKEN_DEF)
            define_new_function_ex(0);
        else
            syntax_error("expected func inside default block", current_token);
    }
    consume(TOKEN_RIGHTBRACE, "expected '}'");

    Dict *dict = malloc(sizeof(Dict));
    dict->capacity = scope_stack[scope_depth].count + 4;
    dict->count = 0;
    dict->entries = malloc((size_t)dict->capacity * sizeof(DictEntry));

    Scope *sc = &scope_stack[scope_depth];
    for (int i = 0; i < sc->count; i++)
        dict_set(dict, sc->vars[i]->name, sc->vars[i]);

    pop_scope();

    Value val = {0};
    val.name = block_name;
    val.type = VAR_DICT;
    val.value.dict = dict;
    return val;
}

static void set_default_export(Value val)
{
    variable_set("default", val);
    Scope *sc = &scope_stack[scope_depth];
    for (int i = 0; i < sc->count; i++)
        if (strcmp(sc->vars[i]->name, "default") == 0)
            sc->vars[i]->exported = 1;
}

static void set_function_buffers(Value *value, char *buffer)
{
    if (!value)
        return;

    if (value->type == VAR_FUNCTION)
    {
        value->value.func.buffer = buffer;
        return;
    }

    if (value->type == VAR_DICT || value->type == VAR_MODULE)
    {
        Dict *dict = value->value.dict;
        if (!dict)
            return;
        for (int i = 0; i < dict->count; i++)
            set_function_buffers(dict->entries[i].value, buffer);
    }
}

Value load_module(char *name, int line)
{
    char module_name[512];
    normalize_module_name(name, module_name, sizeof(module_name));

    // Check cache
    if (module_is_loaded(module_name))
    {
        return clone_value(*get_cached_module(module_name));
    }

    char filepath[512];
    const char *home = getenv("HOME");
    int has_package = 0;
    {
        FILE *pkg = fopen("package.json", "rb");
        if (pkg) { has_package = 1; fclose(pkg); }
    }

    int found = 0;
    if (has_package)
    {
        // Project with package.json: only look in ./chpm_modules/
        if (try_module_path("chpm_modules", module_name, filepath, sizeof(filepath)))
            found = 1;
        else if (strchr(module_name, '/') == NULL && strchr(module_name, '@') == NULL)
        {
            if (try_scoped_module_path("chpm_modules", module_name, filepath, sizeof(filepath)))
                found = 1;
        }
    }
    else
    {
        // No package.json: search current dir, ./chpm_modules/, then ~/chpm_modules/
        // Try current dir
        char tmp[640];
        snprintf(tmp, sizeof(tmp), "%s.chp", module_name);
        FILE *check = fopen(tmp, "rb");
        if (check) { fclose(check); strcpy(filepath, tmp); found = 1; }
        else
        {
            // Try current dir as directory package
            if (try_package_json_main(module_name, filepath, sizeof(filepath))) { found = 1; }
            else
            {
                snprintf(tmp, sizeof(tmp), "%s/main.chp", module_name);
                check = fopen(tmp, "rb");
                if (check) { fclose(check); strcpy(filepath, tmp); found = 1; }
            }

            if (!found && try_module_path("chpm_modules", module_name, filepath, sizeof(filepath))) { found = 1; }
            else if (!found && strchr(module_name, '/') == NULL && strchr(module_name, '@') == NULL)
            {
                if (try_scoped_module_path("chpm_modules", module_name, filepath, sizeof(filepath)))
                    found = 1;
                else if (home)
                {
                    char global_base[640];
                    snprintf(global_base, sizeof(global_base), "%s/chpm_modules", home);
                    if (try_scoped_module_path(global_base, module_name, filepath, sizeof(filepath)))
                        found = 1;
                }
            }
            else if (!found && home)
            {
                char global_base[640];
                snprintf(global_base, sizeof(global_base), "%s/chpm_modules", home);
                if (try_module_path(global_base, module_name, filepath, sizeof(filepath))) { found = 1; }
            }
        }
    }

    if (!found)
    {
        char msg[640];
        snprintf(msg, sizeof(msg), "Module '%s' not found", module_name);
        syntax_error_line(msg, line);
    }

    // Save interpreter state
    char *saved_input;
    int saved_indx, saved_line, saved_scope;
    Token saved_token;
    save_interpreter_state(&saved_input, &saved_indx, &saved_line, &saved_token, &saved_scope);

    // Read module file
    FILE *f = fopen(filepath, "rb");
    if (!f)
    {
        char msg[640];
        snprintf(msg, sizeof(msg), "Module '%s' not found", module_name);
        syntax_error_line(msg, line);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    input = malloc((size_t)size + 1);
    fread(input, 1, (size_t)size, f);
    input[size] = '\0';
    fclose(f);

    // Reset interpreter position for module
    indx = 0;
    g_line = 1;
    push_scope();

    // Execute module (parse top-level statements)
    jumpBOM();
    forward();
    while (current_token.type != TOKEN_EOF)
    {
        if (current_token.type == TOKEN_ID)
            assignation();
        else if (current_token.type == TOKEN_FUNC || current_token.type == TOKEN_DEF || current_token.type == TOKEN_VOID)
            define_new_function_ex(0);
        else if (current_token.type == TOKEN_CONST ||
                 current_token.type == TOKEN_VAR ||
                 current_token.type == TOKEN_LET ||
                 current_token.type == TOKEN_FLOAT_TYPE ||
                 current_token.type == TOKEN_STRING_TYPE ||
                 current_token.type == TOKEN_BOOL_TYPE)
        {
            forward();
            assignation();
        }
        else if (current_token.type == TOKEN_EXPORT)
        {
            forward();
            if (current_token.type == TOKEN_DEFAULT)
            {
                forward();
                Value val = default_block_starts_here() ? parse_default_block() : expression();
                set_default_export(val);
                if (val.type == VAR_DICT || val.type == VAR_MODULE)
                    free_value_internals(&val);
            }
            else if (current_token.type == TOKEN_FUNC || current_token.type == TOKEN_DEF || current_token.type == TOKEN_VOID)
                define_new_function_ex(1);
            else if (current_token.type == TOKEN_CLASS)
                class_stmt(1);
            else if (current_token.type == TOKEN_ID ||
                     current_token.type == TOKEN_CONST ||
                     current_token.type == TOKEN_VAR ||
                     current_token.type == TOKEN_LET ||
                     current_token.type == TOKEN_FLOAT_TYPE ||
                     current_token.type == TOKEN_STRING_TYPE ||
                     current_token.type == TOKEN_BOOL_TYPE)
            {
                // skip optional keyword (const, var, let, float, etc.)
                if (current_token.type != TOKEN_ID)
                    forward();
                char export_name[64];
                strcpy(export_name, current_token.name);
                assignation();
                Scope *s = &scope_stack[scope_depth];
                for (int i = 0; i < s->count; i++)
                    if (strcmp(s->vars[i]->name, export_name) == 0)
                        s->vars[i]->exported = 1;
            }
            else
                break;
        }
        else if (current_token.type == TOKEN_IF)
            if_stmt();
        else if (current_token.type == TOKEN_WHILE)
            while_stmt();
        else if (current_token.type == TOKEN_FOR)
            for_stmt();
        else if (current_token.type == TOKEN_SWITCH)
            switch_stmt();
        else if (current_token.type == TOKEN_TRY)
            try_stmt();
        else if (current_token.type == TOKEN_THROW)
            throw_stmt();
        else if (current_token.type == TOKEN_CLASS)
            class_stmt(0);
        else if (current_token.type == TOKEN_IMPORT)
            import_stmt();
        else if (current_token.type == TOKEN_FROM)
            from_stmt();
        else
            break;
    }

    // Collect only exported items into a dictionary
    Dict *mod_dict = malloc(sizeof(Dict));
    mod_dict->capacity = scope_stack[scope_depth].count + 4;
    mod_dict->count = 0;
    mod_dict->entries = malloc((size_t)mod_dict->capacity * sizeof(DictEntry));

    Scope *mod_scope = &scope_stack[scope_depth];
    for (int i = 0; i < mod_scope->count; i++)
    {
        Value *var = mod_scope->vars[i];
        // Only include items marked with export keyword
        if (!var->exported)
            continue;
        mod_dict->entries[mod_dict->count].key = strdup(var->name);
        mod_dict->entries[mod_dict->count].value = malloc(sizeof(Value));
        *(mod_dict->entries[mod_dict->count].value) = clone_value(*var);
        mod_dict->count++;
    }

    // Capture closure: all module scope variables (private + exported)
    Dict *closure_dict = malloc(sizeof(Dict));
    closure_dict->capacity = mod_scope->count + 4;
    closure_dict->count = 0;
    closure_dict->entries = malloc((size_t)closure_dict->capacity * sizeof(DictEntry));
    for (int i = 0; i < mod_scope->count; i++)
    {
        Value *var = mod_scope->vars[i];
        closure_dict->entries[closure_dict->count].key = strdup(var->name);
        closure_dict->entries[closure_dict->count].value = malloc(sizeof(Value));
        *(closure_dict->entries[closure_dict->count].value) = clone_value(*var);
        closure_dict->count++;
    }

    // Attach closure to exported function values
    for (int i = 0; i < mod_dict->count; i++)
    {
        set_function_buffers(mod_dict->entries[i].value, input);
        if (mod_dict->entries[i].value->type == VAR_FUNCTION)
            mod_dict->entries[i].value->value.func.closure = clone_dict(closure_dict);
    }

    // Free the temporary closure dict (each function got its own clone)
    for (int i = 0; i < closure_dict->count; i++)
    {
        free(closure_dict->entries[i].key);
        if (closure_dict->entries[i].value)
        {
            free_value_internals(closure_dict->entries[i].value);
            free(closure_dict->entries[i].value);
        }
    }
    free(closure_dict->entries);
    free(closure_dict);

    // Clean up module scope
    pop_scope();

    // Keep module buffer alive (functions reference it via start index)
    if (module_buffer_count < MAX_MODULE_BUFFERS)
        module_buffers[module_buffer_count++] = input;
    else
        free(input);

    // Restore interpreter state
    restore_interpreter_state(saved_input, saved_indx, saved_line, saved_token, saved_scope);

    // Create module value
    Value mod_val = {0};
    mod_val.type = VAR_MODULE;
    mod_val.value.dict = mod_dict;

    // Cache module
    cache_module(module_name, mod_val);

    return mod_val;
}
