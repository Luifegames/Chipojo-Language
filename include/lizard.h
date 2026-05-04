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
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_ELIF,
    TOKEN_LEFTBRACE,
    TOKEN_RIGHTBRACE,
    TOKEN_NUM,
    TOKEN_STRING,
    TOKEN_PRINT,
    TOKEN_ASIGN,
    TOKEN_SUM,
    TOKEN_REST,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_EQ,           //  ==
    TOKEN_NE,           //\ !=
    TOKEN_LT,           //  <
    TOKEN_GT,           //  >
    TOKEN_LE,           //  <=
    TOKEN_GE,           //  >=
    TOKEN_LEFTPARENT,   //  (
    TOKEN_RIGHTPARENT,  //  )
    TOKEN_INC,          //\ ++
    TOKEN_DEC,          //\ --
    TOKEN_PLUS_ASSIGN,  //\ +=
    TOKEN_MINUS_ASSIGN, //\ -=
    TOKEN_MULT_ASSIGN,  //\ *=
    TOKEN_DIV_ASSIGN,   //\ /=
    TOKEN_ERROR,
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