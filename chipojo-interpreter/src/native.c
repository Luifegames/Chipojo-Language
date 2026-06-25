#include "native.h"
#include "parser.h"
#include <math.h>

Value native_print(Value *args, int arg_count, int line)
{
    (void)line;
    for (int i = 0; i < arg_count; i++)
    {
        if (args[i].type == VAR_NUMBER)
            printf("%g", args[i].value.num);
        else if (args[i].type == VAR_STRING)
            printf("%s", args[i].value.str);
        else if (args[i].type == VAR_NULL)
            printf("null");
        else if (args[i].type == VAR_DICT)
            dict_print(args[i].value.dict);
        else if (args[i].type == VAR_LIST)
            list_print(args[i].value.list);
        else if (args[i].type == VAR_FUNCTION)
            printf("<func %s>", args[i].name ? args[i].name : "?");
        else if (args[i].type == VAR_MODULE)
            printf("<module>");
        else if (args[i].type == VAR_NATIVE)
            printf("<native>");
        else
            printf("<unknown>");
        if (i < arg_count - 1)
            printf(" ");
    }
    printf("\n");
    Value v = {0};
    v.type = VAR_NULL;
    return v;
}

static Value native_abs(Value *args, int arg_count, int line);

Value native_abs(Value *args, int arg_count, int line)
{
    if (arg_count == 0)
    {
        syntax_error_line("Few argument in abs", line);
    }

    if (arg_count > 1)
    {
        syntax_error_line("Too much argument in abs", line);
    }

    Value result = args[0];
    if (result.type != VAR_NUMBER){
        char message[256];
        sprintf(message,"Var %s is not a number",result.name);
        syntax_error_line(message,line);
    }

    result.value.num = fabs(result.value.num);

    return result;
}

static Value native_input(Value *args, int arg_count, int line)
{
    (void)line;
    if (arg_count > 0 && args[0].type == VAR_STRING)
        printf("%s", args[0].value.str);
    char buf[1024];
    if (fgets(buf, sizeof(buf), stdin))
    {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
        Value v = {0};
        v.type = VAR_STRING;
        v.value.str = strdup(buf);
        return v;
    }
    Value v = {0};
    v.type = VAR_NULL;
    return v;
}

static Value native_number(Value *args, int arg_count, int line)
{
    if (arg_count == 0 || args[0].type != VAR_STRING)
    {
        syntax_error_line("number() expects a string argument", line);
    }
    Value v = {0};
    v.type = VAR_NUMBER;
    v.value.num = atof(args[0].value.str);
    return v;
}

static NativeFunctions natives[] = {
    {"show", native_print},
    {"abs", native_abs},
    {"input", native_input},
    {"number", native_number},
    {NULL, NULL}};

void register_natives()
{
    for (int i = 0; natives[i].name; i++)
    {
        Value v = {0};
        v.type = VAR_NATIVE;
        v.value.native_func = natives[i].func;
        variable_set(natives[i].name, v);
    }
}