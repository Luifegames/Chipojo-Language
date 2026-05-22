#include "variables.h"

#define MAX_VARS 100

Scope scope_stack[MAX_SCOPE];
int scope_depth = 0;

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

    strcpy(new_var->name, name);
    new_var->type = VAR_DICT;
    new_var->value.dict = dict;
    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;

    sc->vars[sc->count] = new_var;
    sc->count++;
}

void dict_set(Dict *dict, char *key, Value *val){
    
    for (int i = 0; i < dict->count; i++)
    {
        if (strcmp(dict ->entries[i].key, key) == 0)
        {

            *(dict->entries[i].value) = *val;

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
    *(dict->entries[dict->count].value) = *val; 
    dict->count++;
}

Value dict_get(Dict *d, char *key)
{
    for (int i = 0; i < d->count; i++)
    {
        if (strcmp(d->entries[i].key, key) == 0)
        {
            Value v = *d->entries[i].value;
            return v;
        }
    }
    Value null_val;
    null_val.type = VAR_NULL;
    return null_val;
}

void assignNumberVar(char *name, double num)
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

    strcpy(new_var->name, name);
    new_var->type = VAR_NUMBER;
    new_var->value.num = num;

    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;

    sc->vars[sc->count] = new_var;
    sc->count++;
}

void assignStringVar(char *name, char *str_value)
{
    Scope *sc = &scope_stack[scope_depth];

    for (int i = 0; i < sc->count; i++)
        if (strcmp(sc->vars[i]->name, name) == 0)
        {
            sc->vars[i]->type = VAR_STRING;
            strcpy(sc->vars[i]->value.str, str_value);
            return;
        }

    Value *new_var = malloc(sizeof(Value));

    strcpy(new_var->name, name);
    new_var->type = VAR_STRING;
    strcpy(new_var->value.str, str_value);

    new_var->value.func.param = NULL;
    new_var->value.func.param_count = 0;

    sc->vars[sc->count] = new_var;
    sc->count++;

}

void function_definition(char *name, int start,char **params, int param_count)
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
    strcpy(new_var->name, name);
    new_var->type = VAR_FUNCTION;
    new_var->value.func.start = start;

    new_var->value.func.param = malloc(param_count * sizeof(char *));
    for (int i = 0; i < param_count; i++)
        new_var->value.func.param[i] = strdup(params[i]);

    new_var->value.func.param_count = param_count;
    sc->vars[sc->count] = new_var;
    sc->count++;

}

void assignNullVar(char *name)
{
    Scope *sc = &scope_stack[scope_depth];
    for (int i = 0; i < sc->count; i++)

        if (strcmp(sc->vars[i]->name, name) == 0)
        {
            sc->vars[i]->type = VAR_NULL;
            return;
        }

    Value *new_var = malloc(sizeof(Value));
    strcpy(new_var->name, name);
    new_var->type = VAR_NULL;
    sc->vars[sc->count] = new_var;
    sc->count++;
    }

Value getVarValue(char *name)
{
    Value v = {0};
    for (int d = scope_depth; d >= 0; d--)
    {
        Scope *sc = &scope_stack[d];
        for (int i = 0; i < sc->count; i++)
            if (strcmp(sc->vars[i]->name, name) == 0)
            {
                strcpy(v.name, sc->vars[i]->name);

                v.type = sc->vars[i]->type;
                if (v.type == VAR_NUMBER)
                    v.value.num = sc->vars[i]->value.num;
                else if (v.type == VAR_STRING)
                    strcpy(v.value.str, sc->vars[i]->value.str);
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
                else if (v.type == VAR_NULL)
                {
                    strcpy(v.name, "null");
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

void setVariable(char *name, Value value){
    
    if (value.type == VAR_STRING)
    {
        assignStringVar(name, value.value.str);
        }
    else if (value.type == VAR_NULL)
    {
        assignNullVar(name);
    }
    else if (value.type == VAR_NUMBER)
    {
        assignNumberVar(name, value.value.num);
    }
    else if (value.type == VAR_DICT)
    {
        dict_new(name, value.value.dict);
    }
    else if (value.type == VAR_NATIVE)
    {
        Scope *sc = &scope_stack[scope_depth];
        Value *new_var = malloc(sizeof(Value));

        strcpy(new_var->name, name);
        new_var->type = VAR_NATIVE;
        new_var->value = value.value;
        sc->vars[sc->count] = new_var;
        sc->count++;
    }
    else{
        undefined_variable_error(name, current_token.line);
    }
}

void pushScope(){
    if (scope_depth + 1 >= MAX_SCOPE)
    {
        runtime_error("Maximum scope depth exceeded");
    }
    scope_depth++;

    scope_stack[scope_depth].count = 0;
}

void popScope()
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
            if (sc->vars[i]->type == VAR_DICT)
            {
                for (int j = 0; j < sc->vars[i]->value.dict->count; j++){
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