#ifndef LIZARD_H
#define LIZARD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Tokens
typedef enum
{
    TOKEN_ID,
    TOKEN_NUM,
    TOKEN_STRING,
    TOKEN_PRINT,
    TOKEN_ASIGN,
    TOKEN_ERROR,
    TOKEN_RIGHTPARENT,
    TOKEN_LEFTPARENT,
    TOKEN_EOF
} TypeToken;

typedef struct
{
    TypeToken type;
    int value;
    char name[64];
} Token;

// Variables
typedef enum
{
    VAR_INT,
    VAR_STRING
} VarType;

typedef struct
{
    char name[64];
    VarType type;
    union
    {
        int int_val;
        char str_val[256];
    } value;
} Var;

extern char *input;
extern int indx;
extern Token current_token;
extern Var vars_table[];
extern int num_vars;

#endif