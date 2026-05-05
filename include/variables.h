#ifndef VARIABLES_H
#define VARIABLES_H

#include "lizard.h"
#include "error.h"

// Variables
typedef enum
{
    VAR_NUMBER,
    VAR_STRING,
    VAR_NULL
} VarType;

typedef struct
{
    char name[64];
    VarType type;
    union
    {
        double val;
        char str_val[256];

    } value;
} Var;


typedef struct
{
    VarType type;
    union
    {
        double num;
        char str[256];
    } value;
} Value;

void assignNumberVar(char *name, double val);
void assignStringVar(char *name, char *val);
void assignNullVar(char *name);
Value getVarValue(char *name);

extern Var vars_table[];
extern int num_vars;

#endif