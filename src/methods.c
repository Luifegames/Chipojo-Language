#include "methods.h"
#include "error.h"

#include <ctype.h>

Value string_upper(Value *args, int arg_count, int line)
{
    Value self = args[0];
    Value result = {0};

    result.type = VAR_STRING;

    strcpy(result.value.str, self.value.str);

    for (int i = 0; result.value.str[i]; i++)
    {
        result.value.str[i] =
            toupper(result.value.str[i]);
    }

    return result;
}

Value dict_size(Value *args, int arg_count, int line)
{
    Value self = args[0];
    Value result = {0};
    result.type = VAR_NUMBER;
    result.value.num =
        self.value.dict->count;
    return result;
}

MethodEntry string_methods[] = {
    {"upper", string_upper},
    {NULL, NULL}};

MethodEntry dict_methods[] = {
    {"size", dict_size},
    {NULL, NULL}};

Value call_method(Value object,char *method,Value *args,int arg_count,int line)
{
    MethodEntry *table = NULL;
    char message[64];
    
    switch (object.type)
    {
    case VAR_STRING:
        table = string_methods;
        break;

    case VAR_DICT:
        table = dict_methods;
        break;

    default:
        sprintf(message, "Type has no methods, type %d", object.type);
        syntax_error_line(message,line);
    }

    for (int i = 0; table[i].name != NULL; i++)
    {
        if (strcmp(table[i].name, method) == 0)
        {
            Value final_args[100];

            final_args[0] = object;

            for (int j = 0; j < arg_count; j++)
            {
                final_args[j + 1] = args[j];
            }

            return table[i].func(
                final_args,
                arg_count + 1,
                line);
        }
    }
    sprintf(message, "Method not found, type %d",object.type);
    syntax_error_line(message, line);

    Value null_val = {0};
    null_val.type = VAR_NULL;
    return null_val;
}