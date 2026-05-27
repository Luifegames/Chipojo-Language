#ifndef VARIABLES_H
#define VARIABLES_H

#include "lizard.h"
#include "error.h"

#define MAX_SCOPE 1000

typedef struct Value Value; // Declaración forward
typedef Value (*NativeFunction)(Value *args, int arg_count, int line);

// Variables
typedef enum
{
    VAR_NUMBER,
    VAR_STRING,
    VAR_FUNCTION,
    VAR_DICT,
    VAR_LIST,
    VAR_NATIVE,
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

//List
typedef struct{
    Value *items;
    int count;
    int capacity;
}List;


//Scope
typedef struct
{
    Value *vars[MAX_SCOPE];
    int count;

} Scope;

// Values
typedef struct Value
{
    char *name;
    VarType type;
    union
    {
        double num;
        char *str;
        Dict *dict;
        List *list;

    struct
    {
        int start;
        char **param;
        int param_count;
    } func;



    NativeFunction native_func;
    } value;
} Value;

void assign_number_val(char *name, double val);
void dict_set(Dict *dict, char *key, Value *val);
Value dict_get(Dict *d, char *key);
void list_push(List *list, Value val);
void dict_new(char *name, Dict *dict);
void list_new(char *name, List *list);
void assign_string_val(char *name, char *val);
void define_function(char *name, int start,char** params,int param_count);
void assign_null_val(char *name);
Value var_value_get(char *name);
// Value getFunction(char *name);
Value list_get(List *list, int index);
Value clone_value(Value v);
void variable_set(char *name, Value value);
void push_scope();
void pop_scope();

extern Scope scope_stack[MAX_SCOPE];
extern int scope_depth;

#endif