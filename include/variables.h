#ifndef VARIABLES_H
#define VARIABLES_H

#include "lizard.h"
#include "error.h"
#define MAX_SCOPE 1000

typedef struct Value Value; // Declaración forward


// Variables
typedef enum
{
    VAR_NUMBER,
    VAR_STRING,
    VAR_FUNCTION,
    VAR_DICT,
    VAR_NULL
} VarType;


//Dictionary
typedef struct 
{
    
    char* key;
    Value* value;
} DictEntry;

typedef struct
{
    DictEntry *entries;   
    int count;
    int capacity;
} Dict;

//Scope
typedef struct
{
    Value *vars[MAX_SCOPE];
    int count;

} Scope;

// Values
typedef struct Value
{
    char name[64];
    VarType type;
    union
    {
        double num;
        char str[256];
        Dict *dict;
    } value;

    struct
    {
        int start;
        char **param;
        int param_count;
    } func;

} Value;

void assignNumberVar(char *name, double val);
void dict_set(Dict *dict, char *key, Value *val);
Value dict_get(Dict *d, char *key);
void dict_new(char *name, Dict *dict);
void assignStringVar(char *name, char *val);
void function_definition(char *name, int start,char** params,int param_count);
void assignNullVar(char *name);
Value getVarValue(char *name);
Value getFunction(char *name);
void setVariable(char *name, Value value);
void pushScope();
void popScope();

extern Scope scope_stack[MAX_SCOPE];
extern int scope_depth;

#endif