#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static const char *token_to_string(TokenType type); // forward decl

#define ERROR_TRY_STACK_MAX 32

static jmp_buf *try_stack[ERROR_TRY_STACK_MAX];
static int try_stack_count = 0;
static char last_error_message[512];

static void throw_to_try(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(last_error_message, sizeof(last_error_message), fmt, args);
    va_end(args);

    if (try_stack_count > 0)
        longjmp(*try_stack[try_stack_count - 1], 1);

    printf("%s\n", last_error_message);
    exit(1);
}

void error_try_push(jmp_buf *env)
{
    if (try_stack_count >= ERROR_TRY_STACK_MAX)
    {
        printf("Chipojo Error: too many nested try blocks\n");
        exit(1);
    }
    try_stack[try_stack_count++] = env;
}

void error_try_pop(void)
{
    if (try_stack_count > 0)
        try_stack_count--;
}

const char *error_last_message(void)
{
    return last_error_message;
}

static const char *token_to_string(TokenType type)
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
    case TOKEN_DEF:
        return "'def'";
    case TOKEN_IMPORT:
        return "'import'";
    case TOKEN_FROM:
        return "'from'";
    case TOKEN_AS:
        return "'as'";
    case TOKEN_EXPORT:
        return "'export'";
    case TOKEN_CONST:
        return "'const'";
    case TOKEN_VAR:
        return "'var'";
    case TOKEN_LET:
        return "'let'";
    case TOKEN_FLOAT_TYPE:
        return "'float'";
    case TOKEN_STRING_TYPE:
        return "'string'";
    case TOKEN_BOOL_TYPE:
        return "'bool'";
    case TOKEN_DEFAULT:
        return "'default'";
    case TOKEN_FOR:
        return "'for'";
    case TOKEN_SWITCH:
        return "'switch'";
    case TOKEN_CASE:
        return "'case'";
    case TOKEN_TRY:
        return "'try'";
    case TOKEN_CATCH:
        return "'catch'";
    case TOKEN_THROW:
        return "'throw'";
    case TOKEN_CLASS:
        return "'class'";
    case TOKEN_PUBLIC:
        return "'public'";
    case TOKEN_PRIVATE:
        return "'private'";
    case TOKEN_VOID:
        return "'void'";
    case TOKEN_SEMICOLON:
        return "';'";
    case TOKEN_ARROW:
        return "'=>'";
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

void __attribute__((noreturn)) syntax_error_line(const char *message, int line){
    throw_to_try("Chipojo Error in line %d : %s", line, message);
    exit(1);
}

    void __attribute__((noreturn)) syntax_error(const char *message, Token token)
{
    char detail[256];
    if (token.type == TOKEN_ID)
        snprintf(detail, sizeof(detail), " ('%s')", token.name);
    else if (token.type == TOKEN_NUM)
        snprintf(detail, sizeof(detail), " (%g)", token.value);
    else if (token.type == TOKEN_STRING)
        snprintf(detail, sizeof(detail), " (\"%s\")", token.name);
    else
        snprintf(detail, sizeof(detail), " (\"%s\")", token_to_string(token.type));

    throw_to_try("Chipojo Error in line %d : %s. Found %s%s", token.line, message, token_to_string(token.type), detail);
    exit(1);
}

void __attribute__((noreturn)) runtime_error(const char *message)
{
    throw_to_try("%s", message);
    exit(1);
}

void __attribute__((noreturn)) chipojo_throw(const char *message)
{
    throw_to_try("%s", message);
    exit(1);
}

void __attribute__((noreturn)) undefined_variable_error(const char *var_name,int line)
{
    throw_to_try("Chipojo Error in line %d : '%s' Undefined value in this scope", line, var_name);
    exit(1);
}

void __attribute__((noreturn)) type_error(const char *var_name, const char *expected, const char *found, int line)
{
    throw_to_try("Chipojo Error in line %d : variable '%s' is type %s, expected %s", line, var_name, found, expected);
    exit(1);
}
