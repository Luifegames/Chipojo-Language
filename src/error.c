#include "error.h"
#include <stdio.h>
#include <stdlib.h>

const char *token_to_string(TokenType type)
{
    switch (type)
    {
    case TOKEN_ID:
        return "identificator";
    case TOKEN_NUM:
        return "number";
    case TOKEN_STRING:
        return "string";
    case TOKEN_PRINT:
        return "'print'";
    case TOKEN_IF:
        return "'if'";
    case TOKEN_ELIF:
        return "'elif'";
    case TOKEN_ELSE:
        return "'else'";
    case TOKEN_WHILE:
        return "'while'";
    case TOKEN_ASIGN:
        return "'='";
    case TOKEN_SUM:
        return "'+'";
    case TOKEN_REST:
        return "'-'";
    case TOKEN_MUL:
        return "'*'";
    case TOKEN_DIV:
        return "'/'";
    case TOKEN_INC:
        return "'++'";
    case TOKEN_DEC:
        return "'--'";
    case TOKEN_PLUS_ASSIGN:
        return "'+='";
    case TOKEN_MINUS_ASSIGN:
        return "'-='";
    case TOKEN_MULT_ASSIGN:
        return "'*='";
    case TOKEN_DIV_ASSIGN:
        return "'/='";
    case TOKEN_PARENTLEFT:
        return "'('";
    case TOKEN_PARENTRIGHT:
        return "')'";
    case TOKEN_LEFTBRACE:
        return "'{'";
    case TOKEN_RIGHTBRACE:
        return "'}'";
    case TOKEN_EQ:
        return "'=='";
    case TOKEN_NE:
        return "'!='";
    case TOKEN_LT:
        return "'<'";
    case TOKEN_GT:
        return "'>'";
    case TOKEN_LE:
        return "'<='";
    case TOKEN_GE:
        return "'>='";       
    case TOKEN_DOT:
        return ".";
    case TOKEN_COMMA:
        return ",";
    case TOKEN_COLON:
        return ":";
    case TOKEN_ERROR:
        return "error";
    case TOKEN_RETURN:
        return "return";
    case  TOKEN_LEFTBRACKET:
        return "[";
    case TOKEN_RIGHTBRACKET:
        return "]";
    case TOKEN_EOF:
        return "file end";
    default:
        return "unknown";
    }
}

void syntax_error_line(const char *message, int line){
    printf("Chipojo Error in line %d : %s \n", line, message);
    exit(1);
}

    void syntax_error(const char *message, Token token)
{
    printf("Chipojo Error in line %d : %s. Found %s",token.line, message, token_to_string(token.type));
    if (token.type == TOKEN_ID)
    {
        printf(" ('%s')", token.name);
    }
    else if (token.type == TOKEN_NUM)
    {
        printf(" (%g)", token.value);
    }
    else if (token.type == TOKEN_STRING)
    {
        printf(" (\"%s\")", token.name);
    }
    else{
        printf(" (\"%s\")", token_to_string(token.type));
    }
    printf("\n");
    exit(1);
}

void runtime_error(const char *message)
{
    printf("%s",message);
    exit(1);
}

void undefined_variable_error(const char *var_name,int line)
{
    printf("Chipojo Error in line %d : '%s' Undefined value in this scope",line, var_name);
    exit(1);
}

void type_error(const char *var_name, const char *expected, const char *found, int line)
{
    printf("Chipojo Error in line %d : variable '%s' is type %s, expected %s\n",line, var_name, found, expected);
    exit(1);
}