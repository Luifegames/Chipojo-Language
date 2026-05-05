#include "variables.h"

#define MAX_VARS 100
Var vars_table[MAX_VARS];
int num_vars = 0;

void assignNumberVar(char *name, double val)
{
    for (int i = 0; i < num_vars; i++)
        if (strcmp(vars_table[i].name, name) == 0)
        {
            vars_table[i].type = VAR_NUMBER;
            vars_table[i].value.val = val;
            return;
        }
    strcpy(vars_table[num_vars].name, name);
    vars_table[num_vars].type = VAR_NUMBER;
    vars_table[num_vars].value.val = val;
    num_vars++;
}

void assignStringVar(char *name, char *val)
{
    for (int i = 0; i < num_vars; i++)
        if (strcmp(vars_table[i].name, name) == 0)
        {
            vars_table[i].type = VAR_STRING;
            strcpy(vars_table[i].value.str_val, val);
            return;
        }
    strcpy(vars_table[num_vars].name, name);
    vars_table[num_vars].type = VAR_STRING;
    strcpy(vars_table[num_vars].value.str_val, val);
    num_vars++;
}

void assignNullVar(char *name)
{
    for (int i = 0; i < num_vars; i++)
        if (strcmp(vars_table[i].name, name) == 0)
        {
            vars_table[i].type = VAR_NULL;
            return;
        }
    strcpy(vars_table[num_vars].name, name);
    vars_table[num_vars].type = VAR_NULL;
    num_vars++;
}

Value getVarValue(char *name)
{
    Value v;
    for (int i = 0; i < num_vars; i++)
    {
        if (strcmp(vars_table[i].name, name) == 0)
        {
            v.type = vars_table[i].type;
            if (v.type == VAR_NUMBER)
                v.value.num = vars_table[i].value.val;
            if (v.type == VAR_STRING)
                strcpy(v.value.str, vars_table[i].value.str_val);
            return v;
        }
    }
    undefined_variable_error(name, current_token.line);
    return v;
}