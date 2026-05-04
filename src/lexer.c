#include "lexer.h"
char *input = NULL;
int indx = 0;
int g_line = 1;

void nextChar(void) {
    if (input[indx] == '\n'){
        g_line++;
    }
    indx++;
}
char currentChar(void) { return input[indx]; }
static char peekChar(void) { return input[indx + 1]; }

void jumpBlankspace(void)
{
    while (isspace((unsigned char)currentChar()))
        nextChar();
}

void jumpBOM(void)
{
    if ((unsigned char)input[0] == 0xEF && (unsigned char)input[1] == 0xBB && (unsigned char)input[2] == 0xBF)
    {
        indx = 3;
    }
}

TypeToken peek_next_token_type(void)
{
    int saved_pos = indx;
    Token t = nextToken();
    TypeToken type = t.type;
    indx = saved_pos;
    return type;
}

Token nextToken()
{
    jumpBlankspace();
    Token t;
    t.line = g_line;
    char c = currentChar();
    if (c == '\0')
    {
        t.type = TOKEN_EOF;
        return t;
    }

    // Identificators and key word
    if (isalpha((unsigned char)c))
    {
        int i = 0;
        while (isalnum((unsigned char)currentChar()) && i < 63)
        {
            t.name[i++] = currentChar();
            nextChar();
        }
        t.name[i] = '\0';
        if (strcmp(t.name, "print") == 0)
            t.type = TOKEN_PRINT;
        else if (strcmp(t.name, "if") == 0)
            t.type = TOKEN_IF;
        else if (strcmp(t.name, "else") == 0)
            t.type = TOKEN_ELSE;
        else if (strcmp(t.name, "elif") == 0)
            t.type = TOKEN_ELIF;
        else if (strcmp(t.name, "while") == 0)
            t.type = TOKEN_WHILE;
        else if (strcmp(t.name, "true") == 0)
            t.type = TOKEN_TRUE;
        else if (strcmp(t.name, "false") == 0)
            t.type = TOKEN_FALSE;
        else
            t.type = TOKEN_ID;
        return t;
    }
    // Number
    if (isdigit((unsigned char)c))
    {
        t.value = 0;
        while (isdigit((unsigned char)currentChar()))
        {
            t.value = t.value * 10 + (currentChar() - '0');
            nextChar();
        }
        t.type = TOKEN_NUM;
        return t;
    }

    // String
    if (c == '"')
    {
        nextChar();
        int i = 0;
        while (currentChar() != '"' && currentChar() != '\0')
        {
            if (i < 63)
                t.name[i++] = currentChar();
            nextChar();
        }
        if (currentChar() == '"')
        {
            nextChar();
            t.name[i] = '\0';
            t.type = TOKEN_STRING;
        }
        else
        {
            t.type = TOKEN_ERROR;
            printf("Error: cadena no cerrada\n");
        }
        return t;
    }
    // Operator
    if (c == '=' && peekChar() == '=')
    {
        t.type = TOKEN_EQ;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '+' && peekChar() == '+')
    {
        t.type = TOKEN_INC;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '+' && peekChar() == '=')
    {
        t.type = TOKEN_PLUS_ASSIGN;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '-' && peekChar() == '=')
    {
        t.type = TOKEN_MINUS_ASSIGN;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '*' && peekChar() == '=')
    {
        t.type = TOKEN_MULT_ASSIGN;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '/' && peekChar() == '=')
    {
        t.type = TOKEN_DIV_ASSIGN;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '-' && peekChar() == '-')
    {
        t.type = TOKEN_DEC;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '!' && peekChar() == '=')
    {
        t.type = TOKEN_NE;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '<' && peekChar() == '=')
    {
        t.type = TOKEN_LE;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '>' && peekChar() == '=')
    {
        t.type = TOKEN_GE;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '<')
    {
        t.type = TOKEN_LT;
        nextChar();
        nextChar();
        return t;
    }

    if (c == '>')
    {
        t.type = TOKEN_GT;
        nextChar();
        nextChar();
        return t;
    }

    // Sign
    switch (c)
    {
    case '=':
        t.type = TOKEN_ASIGN;
        break;
    case '+':
        t.type = TOKEN_SUM;
        break;
    case '-':
        t.type = TOKEN_REST;
        break;
    case '*':
        t.type = TOKEN_MUL;
        break;
    case '/':
        t.type = TOKEN_DIV;
        break;
    case '(':
        t.type = TOKEN_PARENTLEFT;
        break;
    case ')':
        t.type = TOKEN_PARENTRIGHT;
        break;
    case '{':
        t.type = TOKEN_LEFTBRACE;
        break;
    case '}':
        t.type = TOKEN_RIGHTBRACE;
        break;
    default:
        nextChar();
        return nextToken();
    }
    nextChar();
    return t;
}