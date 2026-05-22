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
    TOKEN_EQ,          //  ==
    TOKEN_NE,          //\ !=
    TOKEN_LT,          //  <
    TOKEN_GT,          //  >
    TOKEN_LE,          //  <=
    TOKEN_GE,          //  >=
    TOKEN_PARENTLEFT,  //  (
    TOKEN_PARENTRIGHT, //  )
    TOKEN_WHILE,
    TOKEN_INC,          //\ ++
    TOKEN_DEC,          //\ --
    TOKEN_PLUS_ASSIGN,  //\ +=
    TOKEN_MINUS_ASSIGN, //\ -=
    TOKEN_MULT_ASSIGN,  //\ *=
    TOKEN_DIV_ASSIGN,   //\ /=
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_FUNC,
    TOKEN_RETURN,
    TOKEN_COMMA,
    TOKEN_HAS,          // has
    TOKEN_DOT,          // .
    TOKEN_COLON,        // :
    TOKEN_NULL,         // null
    TOKEN_AND,          // and
    TOKEN_OR,           // or
    TOKEN_NOT,          // not
    TOKEN_ERROR,
    TOKEN_EOF
} TokenType;

typedef struct
{
    TokenType type;
    double value;
    char name[64];
    int line;
} Token;

extern int g_line;



extern char *input;
extern int indx;
extern Token current_token;

#endif