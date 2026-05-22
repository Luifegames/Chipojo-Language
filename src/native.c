#include "native.h"
#include "parser.h"

Value native_print(Value *args, int arg_count, int line)
{
    for (int i = 0; i < arg_count; i++)
    {
        if (args[i].type == VAR_NUMBER)
            printf("%g", args[i].value.num);
        else if (args[i].type == VAR_STRING)
            printf("%s", args[i].value.str);
        else if (args[i].type == VAR_NULL)
            printf("null");
        else if (args[i].type == VAR_DICT)
        {
            dict_print(args[i].value.dict);
        }
        if (i < arg_count - 1)
            printf(" ");
    }
    printf("\n");
    Value v = {0};
    strcpy(v.name, "Printer");
    v.type = VAR_NULL;
    return v;
}

Value native_abs(Value *args, int arg_count, int line)
{
    Value result = args[0];
    if (result.type != VAR_NUMBER){
        char message[256];
        sprintf(message,"Var %s is not a number",result.name);
        syntax_error_line(message,line);
    }

    result.value.num = abs(result.value.num);

    return result;
}

static NativeFunctions natives[] = {
    {"show", native_print},
    {"abs", native_abs},
    {NULL, NULL}};

void register_natives()
{
    for (int i = 0; natives[i].name; i++)
    {
        Value v = {0};
        v.type = VAR_NATIVE;
        v.value.native_func = natives[i].func;
        setVariable(natives[i].name, v);
    }
}