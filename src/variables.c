#include "variables.h"

#define MAX_VARS 100

Scope scope_stack[MAX_SCOPE];
int scope_depth = 0;

Value clone_value(Value v)
{
    Value copy = {0};

    copy.type = v.type;

    switch (v.type)
    {
    case VAR_NUMBER:
        copy.value.num = v.value.num;
        break;

    case VAR_STRING:
        copy.value.str = strdup(v.value.str);
        break;

    case VAR_DICT:
        copy.value.dict = v.value.dict;
        break;

    case VAR_FUNCTION:
        copy.value.func = v.value.func;
        break;

    case VAR_NATIVE:
        copy.value.native_func = v.value.native_func;
        break;
    case VAR_LIST:
        copy.value.list = v.value.list;
        break;

    case VAR_NULL:
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
        list->items = realloc(list->items, sizeof(Value) * list->capacity);
    }

    list->items[list->count++] = clone_value(val);
}

void dict_new(char *name, Dict *dict){
    Scope *sc = &scope_stack[scope_depth];

    for (int i = 0; i < sc->count; i++)
        if (strcmp(sc->vars[i]->name, name) == 0)
        {
            sc->vars[i]->type = VAR_DICT;
            sc->vars[i]->value.dict = dict;

            return;
        }
    Value *new_var = malloc(sizeof(Value));

    new_var->name = strdup(name);
    new_var->type = VAR_DICT;
    new_var->value.dict = dict;
    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;

    sc->vars[sc->count++] = new_var;
}

void list_new(char *name, List *list)
{
    Scope *sc = &scope_stack[scope_depth];

    for (int i = 0; i < sc->count; i++)
        if (strcmp(sc->vars[i]->name, name) == 0)
        {
            sc->vars[i]->type = VAR_LIST;
            sc->vars[i]->value.list = list;

            return;
        }
    Value *new_var = malloc(sizeof(Value));

    new_var->name = strdup(name);
    new_var->type = VAR_LIST;
    new_var->value.list = list;
    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;

    sc->vars[sc->count++] = new_var;
}

void dict_set(Dict *dict, char *key, Value *val){
    
    for (int i = 0; i < dict->count; i++)
    {
        if (strcmp(dict ->entries[i].key, key) == 0)
        {
            if (dict->entries[i].value->type == VAR_STRING)
            {
                free(dict->entries[i].value->value.str);
            }
            *(dict->entries[i].value) = clone_value(*val);

            return;
        }
    }
    
    if (dict->count >= dict->capacity)
    {
        dict->capacity *= 2;
        dict->entries = realloc(dict->entries, dict->capacity * sizeof(DictEntry));
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
    for (int i = 0; i < sc->count; i++)
        if (strcmp(sc->vars[i]->name, name) == 0)
        {
            sc->vars[i]->type = VAR_NUMBER;
            sc->vars[i]->value.num = num;

            return;
        }
    Value *new_var = malloc(sizeof(Value));

    new_var->name = strdup(name);
    new_var->type = VAR_NUMBER;
    new_var->value.num = num;

    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;

    sc->vars[sc->count++] = new_var;
}

void assign_string_val(char *name, char *str_value)
{
    Scope *sc = &scope_stack[scope_depth];

    for (int i = 0; i < sc->count; i++)
        if (strcmp(sc->vars[i]->name, name) == 0)
        {
            sc->vars[i]->type = VAR_STRING;
            free(sc->vars[i]->value.str);
            sc->vars[i]->value.str = strdup(str_value);
            return;
        }

    Value *new_var = malloc(sizeof(Value));

    new_var->name = strdup(name);
    new_var->type = VAR_STRING;
    new_var->value.str = strdup(str_value);

    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;

    sc->vars[sc->count++] = new_var;

}

void define_function(char *name, int start,char **params, int param_count)
{
    Scope *sc = &scope_stack[scope_depth];
    for (int i = 0; i < sc->count; i++)
        if (strcmp(sc->vars[i]->name, name) == 0)
        {
            char error[256];
            snprintf(error, sizeof(error), "Function %s is defined twice", name);
            syntax_error_line(error, current_token.line);
            exit(1);
        }

    Value *new_var = malloc(sizeof(Value));
    new_var->name = strdup(name);
    new_var->type = VAR_FUNCTION;
    new_var->value.func.start = start;

    new_var->value.func.param = malloc(param_count * sizeof(char *));
    for (int i = 0; i < param_count; i++)
        new_var->value.func.param[i] = strdup(params[i]);

    new_var->value.func.param_count = param_count;
    sc->vars[sc->count++] = new_var;

}

void assign_null_val(char *name)
{
    Scope *sc = &scope_stack[scope_depth];
    for (int i = 0; i < sc->count; i++)

        if (strcmp(sc->vars[i]->name, name) == 0)
        {
            sc->vars[i]->type = VAR_NULL;
            return;
        }

    Value *new_var = malloc(sizeof(Value));
    new_var->name = strdup(name);
    new_var->type = VAR_NULL;
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
                v.name = strdup(sc->vars[i]->name);

                v.type = sc->vars[i]->type;
                if (v.type == VAR_NUMBER)
                    v.value.num = sc->vars[i]->value.num;
                else if (v.type == VAR_STRING)
                    v.value.str = strdup(sc->vars[i]->value.str);
                else if (v.type == VAR_FUNCTION)
                {
                    v.value.func.start = sc->vars[i]->value.func.start;
                    v.value.func.param = sc->vars[i]->value.func.param;
                    v.value.func.param_count = sc->vars[i]->value.func.param_count;
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
                    v.name = strdup("null");
                    v.type = VAR_NULL;
                }
                else if (v.type == VAR_NATIVE)
                {
                    v.value.native_func = sc->vars[i]->value.native_func;
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
        dict_new(name, value.value.dict);
    }
    else if (value.type == VAR_LIST)
    {
        list_new(name, value.value.list);
    }
    else if (value.type == VAR_NATIVE)
    {
        Scope *sc = &scope_stack[scope_depth];
        Value *new_var = malloc(sizeof(Value));

        new_var->name = strdup(name);
        new_var->type = VAR_NATIVE;
        new_var->value = value.value;
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
            if (sc->vars[i]->type == VAR_FUNCTION)
            {
                for (int j = 0; j < sc->vars[i]->value.func.param_count; j++)
                    free(sc->vars[i]->value.func.param[j]);
                free(sc->vars[i]->value.func.param);
            }
            
            if (sc->vars[i]->type == VAR_LIST)
            {
                List *list = sc->vars[i]->value.list;

                for (int j = 0; j < list->count; j++)
                {
                    if (list->items[j].type == VAR_STRING)
                    {
                        free(list->items[j].value.str);
                    }
                }

                free(list->items);
                free(list);
            }

                if (sc->vars[i]->type == VAR_DICT)
                {
                    for (int j = 0; j < sc->vars[i]->value.dict->count; j++)
                    {
                        if (sc->vars[i]->type == VAR_STRING)
                        {
                            free(sc->vars[i]->value.str);
                        }
                        free(sc->vars[i]->value.dict->entries[j].key);
                        free(sc->vars[i]->value.dict->entries[j].value);
                    }
                    free(sc->vars[i]->value.dict->entries);
                    free(sc->vars[i]->value.dict);
                }
                free(sc->vars[i]);
            }
        scope_depth--;
    }
}