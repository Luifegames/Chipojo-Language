#ifndef CHIPOJO_H
#define CHIPOJO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CHIPOJO_VERSION "0.7.0"
#define CHIPOJO_AUTHOR "Luife"
#define CHIPOJO_YEAR "2026"

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
    TOKEN_DEF,
    TOKEN_RETURN,
    TOKEN_COMMA,
    TOKEN_DOT,          // .
    TOKEN_COLON,        // :
    TOKEN_NULL,         // null
    TOKEN_AND,          // and
    TOKEN_OR,           // or
    TOKEN_NOT,          // not
    TOKEN_IMPORT,       // import
    TOKEN_FROM,         // from
    TOKEN_AS,           // as
    TOKEN_EXPORT,       // export
    TOKEN_CONST,        // const
    TOKEN_VAR,          // var
    TOKEN_LET,          // let
    TOKEN_FLOAT_TYPE,   // float (type annotation)
    TOKEN_STRING_TYPE,  // string (type annotation)
    TOKEN_BOOL_TYPE,    // bool (type annotation)
    TOKEN_DEFAULT,      // default (export default)
    TOKEN_FOR,          // for
    TOKEN_SWITCH,       // switch
    TOKEN_CASE,         // case
    TOKEN_TRY,          // try
    TOKEN_CATCH,        // catch
    TOKEN_THROW,        // throw
    TOKEN_CLASS,        // class
    TOKEN_PUBLIC,       // public
    TOKEN_PRIVATE,      // private
    TOKEN_VOID,         // void
    TOKEN_SEMICOLON,    // ;
    TOKEN_ARROW,        // =>
    TOKEN_ERROR,
    TOKEN_RIGHTBRACKET, //]
    TOKEN_LEFTBRACKET,  //[
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
