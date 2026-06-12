#ifndef VARIABLES_H
#define VARIABLES_H

#include "chipojo.h"
#include "error.h"

#define MAX_SCOPE 1000

typedef struct Value Value; 
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

int function_exist(char *name);
Value clone_value(Value v);
Value get_dict(Dict *d, char *key);
Value *get_dict_ref(Dict *d, char *key);
Value get_var_value(char *name);
Value *get_var_value_ref(char *name);
Value get_list(List *list, int index);

void assign_number_val(char *name, double val);
void assign_null_val(char *name);
void assign_string_val(char *name, char *val);
void assign_dict_val(char *name, Dict *dict);
void assign_list_val(char *name, List *list);

void set_dict(Dict *dict, char *key, Value *val);
void push_list(List *list, Value val);
void define_function(char *name, int start,char** params,int param_count);
void set_variable(char *name, Value value);
void push_scope();
void pop_scope();

extern Scope scope_stack[MAX_SCOPE];
extern int scope_depth;

#endif