#include "methods.h"
#include "error.h"

#include <ctype.h>

Value string_upper(Value *args, int arg_count, int line)
{

    if (arg_count != 1)
    {
        syntax_error_line("Upper no need argument", line);
    }


    Value self = args[0];
    Value result = {0};

    result.type = VAR_STRING;

    result.value.str =  strdup(self.value.str);

    for (int i = 0; result.value.str[i]; i++)
    {
        result.value.str[i] =
            toupper(result.value.str[i]);
    }

    return result;
}

Value string_lower(Value *args, int arg_count, int line)
{

    if (arg_count != 1)
    {
        syntax_error_line("Lower no need argument", line);
    }

    Value self = args[0];
    Value result = {0};

    result.type = VAR_STRING;

    result.value.str = strdup(self.value.str);

    for (int i = 0; result.value.str[i]; i++)
    {
        result.value.str[i] =
            tolower(result.value.str[i]);
    }

    return result;
}



Value dict_size(Value *args, int arg_count, int line)
{

    if (arg_count != 1)
    {
        syntax_error_line("Size no need argument", line);
    }

    Value self = args[0];
    Value result = {0};
    result.type = VAR_NUMBER;
    result.value.num =
        self.value.dict->count;
    return result;
}

Value dict_has(Value *args, int arg_count, int line)
{

    if (arg_count != 2)
    {
        syntax_error_line("Need 1 argument", line);
    }

    if (args[1].type != VAR_STRING)
    {
        syntax_error_line("Argument in 'has' must be a String", line);
    }

    Value self = args[0];
    char *key = args[1].value.str;
    Value result = {0};
    result.type = VAR_NUMBER;
    result.value.num = 0;
    Value dict_val = dict_get(self.value.dict, key);
    if (dict_val.type != VAR_NULL)
        result.value.num = 1;
    return result;
}

Value dict_getter(Value *args, int arg_count, int line)
{
    if (arg_count != 2)
    {
        syntax_error_line("Need 1 argument", line);
    }

    if (args[1].type != VAR_STRING)
    {
        syntax_error_line("Get argument must be a String", line);
    }

    Value self = args[0];
    char *key = args[1].value.str;
    Value result = dict_get(self.value.dict, key);
    return result;
}

Value dict_setter(Value *args, int arg_count, int line)
{
    if (arg_count != 3)
    {
        syntax_error_line("Need 2 argument", line);
    }

    if (args[1].type != VAR_STRING)
    {
        syntax_error_line("Set first argument must be a String", line);
    }

    Value self = args[0];
    char *key = args[1].value.str;
    Value *val = &args[2];
    dict_set(self.value.dict, key,val);
    Value result = dict_get(self.value.dict, key);
    return result;
}

MethodEntry string_methods[] = {
    {"upper", string_upper},
    {"lower", string_lower},
    {NULL, NULL}};

MethodEntry dict_methods[] = {
    {"size", dict_size},
    {"has", dict_has},
    {"get", dict_getter},
    {"set", dict_setter},
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

            return table[i].func(final_args,arg_count + 1,line);
        }
    }
    sprintf(message, "Method not found, type %d",object.type);
    syntax_error_line(message, line);

    Value null_val = {0};
    null_val.type = VAR_NULL;
    return null_val;
}