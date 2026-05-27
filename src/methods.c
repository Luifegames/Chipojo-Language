#include "methods.h"
#include "error.h"

#include <ctype.h>

Value string_upper(Value *args, int arg_count, int line)
{

    if (arg_count != 1)
    {
        syntax_error_line("upper()", line);
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
        syntax_error_line("lower()", line);
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

Value size_get(Value *args, int arg_count, int line)
{

    if (arg_count != 1)
    {
        syntax_error_line("size()", line);
    }

    Value self = args[0];
    Value result = {0};
    result.type = VAR_NUMBER;
    if (self.type == VAR_DICT){
        result.value.num =self.value.dict->count;
    }
    else if (self.type == VAR_LIST)
    {
        result.value.num =self.value.list->count;
    }
    
    return result;
}

Value dict_has(Value *args, int arg_count, int line)
{

    if (arg_count != 2)
    {
        syntax_error_line("has(value)", line);
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
        syntax_error_line("get(value)", line);
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
        syntax_error_line("set(key,value)", line);
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

Value list_getter(Value *args, int arg_count, int line)
{
    if (arg_count != 2)
    {
        syntax_error_line("Need 1 argument", line);
    }

    if (args[1].type != VAR_NUMBER)
    {
        syntax_error_line("Get argument must be a Number", line);
    }

    Value self = args[0];
    int index = args[1].value.num;
    Value result = list_get(self.value.list, index);
    return result;
}

Value list_add(Value *args, int arg_count, int line)
{
    if (arg_count != 2)
    {
        runtime_error("push() takes 1 arg");
    }
    Value object = args[0];
    list_push(object.value.list, args[1]);

    Value nullv = {0};
    nullv.type = VAR_NULL;

    return nullv;
}

Value list_pop(Value *args, int arg_count, int line)
{
    if (arg_count !=1)
    {
        runtime_error("pop() no need arg");
    }
    Value object = args[0];
    List *list = object.value.list;
    Value v = {0};
    v.type = VAR_NULL;
    if (list->count > 0)
    {
        v = clone_value(
            list->items[list->count - 1]);

        list->count--;
    }
    return v;
}

Value list_is_empty(Value *args, int arg_count, int line)
{
    if (arg_count != 1)
    {
        runtime_error("is_empty() no need arg");
    }
    Value object = args[0];
    Value v = {0};
    v.type = VAR_NUMBER;
    v.value.num = object.value.list->count == 0;
    return v;
}

Value list_insert(Value *args, int arg_count, int line)
{
    if (arg_count != 3)
    {
        runtime_error("insert(index, value)");
    }

    List *list = args[0].value.list;

    if (args[1].type != VAR_NUMBER)
    {
        runtime_error("Index must be number");
    }

    int index = (int)args[1].value.num;

    if (index < 0 || index > list->count)
    {
        runtime_error("Index out of range");
    }

    if (list->count >= list->capacity)
    {
        list->capacity *= 2;

        list->items = realloc(
            list->items,
            sizeof(Value) * list->capacity);
    }

    for (int i = list->count; i > index; i--)
    {
        list->items[i] = list->items[i - 1];
    }

    list->items[index] = clone_value(args[2]);

    list->count++;

    Value v = {0};
    v.type = VAR_NULL;

    return v;
}

Value list_remove(Value *args, int arg_count, int line)
{
    if (arg_count != 2)
    {
        runtime_error("remove(index)");
    }

    List *list = args[0].value.list;

    int index = (int)args[1].value.num;

    if (index < 0 || index >= list->count)
    {
        runtime_error("Index out of range");
    }

    Value removed = clone_value(list->items[index]);

    for (int i = index; i < list->count - 1; i++)
    {
        list->items[i] = list->items[i + 1];
    }

    list->count--;

    return removed;
}

Value list_clear(Value *args, int arg_count, int line)
{
    if (arg_count != 1)
    {
        runtime_error("clear()");
    }

    List *list = args[0].value.list;

    list->count = 0;

    Value v = {0};
    v.type = VAR_NULL;

    return v;
}

int value_equals(Value a, Value b)
{
    if (a.type != b.type)
        return 0;

    switch (a.type)
    {
    case VAR_NUMBER:
        return a.value.num == b.value.num;

    case VAR_STRING:
        return strcmp(a.value.str, b.value.str) == 0;

    case VAR_NULL:
        return 1;

    default:
        return 0;
    }
}

Value list_contains(Value *args, int arg_count, int line)
{
    if (arg_count != 2)
    {
        runtime_error("contains(value)");
    }

    List *list = args[0].value.list;

    for (int i = 0; i < list->count; i++)
    {
        if (value_equals(list->items[i], args[1]))
        {
            Value v = {0};

            v.type = VAR_NUMBER;
            v.value.num = 1;

            return v;
        }
    }

    Value v = {0};

    v.type = VAR_NUMBER;
    v.value.num = 0;

    return v;
}

Value list_find(Value *args, int arg_count, int line)
{
    if (arg_count != 2)
    {
        runtime_error("find(value)");
    }

    List *list = args[0].value.list;

    for (int i = 0; i < list->count; i++)
    {
        if (value_equals(list->items[i], args[1]))
        {
            Value v = {0};

            v.type = VAR_NUMBER;
            v.value.num = i;

            return v;
        }
    }

    Value v = {0};

    v.type = VAR_NUMBER;
    v.value.num = -1;

    return v;
}
Value list_reverse(Value *args, int arg_count, int line)
{
    if (arg_count != 1)
    {
        runtime_error("reverse()");
    }

    List *list = args[0].value.list;

    int start = 0;
    int end = list->count - 1;

    while (start < end)
    {
        Value temp = list->items[start];

        list->items[start] = list->items[end];

        list->items[end] = temp;

        start++;
        end--;
    }

    Value v = {0};

    v.type = VAR_NULL;

    return v;
}
MethodEntry string_methods[] = {
    {"upper", string_upper},
    {"lower", string_lower},
    {NULL, NULL}};

MethodEntry dict_methods[] = {
    {"size", size_get},
    {"has", dict_has},
    {"get", dict_getter},
    {"set", dict_setter},
    {NULL, NULL}};

MethodEntry list_methods[] = {
    {"size", size_get},
    {"get", list_getter},
    {"push", list_add},
    {"pop", list_pop},
    {"is_empty", list_is_empty},
    {"insert", list_insert},
    {"remove", list_remove},
    {"clear", list_clear},
    {"contains", list_contains},
    {"find", list_find},
    {"reverse", list_reverse},
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
    case VAR_LIST:
        table = list_methods;
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