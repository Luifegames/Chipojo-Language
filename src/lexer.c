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


void skipLineComment(void){
    while (currentChar() != '\0' && currentChar() != '\n'){
        nextChar();
    }
}

void skipBlockComment()
{
    int current_line = g_line;
    while (currentChar() != '\0')
    {

        if (currentChar() == '*' && peekChar() == '/')
        {
            nextChar();
            nextChar();
            return;
        }

        if (currentChar() == '\n')
            g_line++;

        nextChar();
    }

    syntax_error_line("Unterminated block comment", current_line);
}

void jumpBOM(void)
{
    if ((unsigned char)input[0] == 0xEF && (unsigned char)input[1] == 0xBB && (unsigned char)input[2] == 0xBF)
    {
        indx = 3;
    }
}

TokenType peek_next_token_type(void){

    int saved_index = indx;
    int saved_line = g_line;

    Token t = nextToken();

    indx = saved_index;
    g_line = saved_line;

    return t.type;
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
    if (isalpha((unsigned char)c) || currentChar() == '_')
    {
        int i = 0;
        while ((isalnum((unsigned char)currentChar()) || currentChar() =='_') && i < 63)
        {
            t.name[i++] = currentChar();
            nextChar();
        }
        t.name[i] = '\0';
        // if (strcmp(t.name, "print") == 0)
        //     t.type = TOKEN_PRINT;
        if (strcmp(t.name, "if") == 0)
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
        else if (strcmp(t.name, "null") == 0)
            t.type = TOKEN_NULL;
        else if (strcmp(t.name, "func") == 0)
            t.type = TOKEN_FUNC;
        else if (strcmp(t.name, "return") == 0)
            t.type = TOKEN_RETURN;
        else if (strcmp(t.name, "and") == 0)
            t.type = TOKEN_AND;
        else if (strcmp(t.name, "or") == 0)
            t.type = TOKEN_OR;
        else if (strcmp(t.name, "not") == 0)
            t.type = TOKEN_NOT;
        else
            t.type = TOKEN_ID;
        return t;
    }
    // Number
    if (isdigit((unsigned char)c))
    {
        double val = 0.0;
        int decimal = 0;
        double frac = 0.1; 

        while (isdigit((unsigned char)currentChar()) || currentChar()=='.')
        {   
            if (currentChar() == '.'){
            if (decimal) break;
            decimal = 1;
            nextChar();
            continue;
        }
        if (!decimal)
        {
            val = val * 10 + (currentChar() - '0');
        }
        else
        {
            val = val + (currentChar() - '0') * frac;
            frac *= 0.1;
        }
        nextChar();
        }
        t.type = TOKEN_NUM;
        t.value = val;
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
            printf("Error: Unclose String\n");
        }
        return t;
    }
    if (c == '/' && peekChar() == '/')
    {
        skipLineComment();
        return nextToken();
    }

    if (c == '/' && peekChar() == '*'){
        skipBlockComment();
        return nextToken();
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
        return t;
    }

    if (c == '>')
    {
        t.type = TOKEN_GT;
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
    case '!':
        t.type = TOKEN_NOT;
        break;
    case '}':
        t.type = TOKEN_RIGHTBRACE;
        break;
    case '.':
        t.type = TOKEN_DOT;
        break;
    case ':':
        t.type = TOKEN_COLON;
        break;
    case ',':
        t.type = TOKEN_COMMA;
        break;
    case '[':
        t.type = TOKEN_LEFTBRACKET;
        break;
    case ']':
        t.type = TOKEN_RIGHTBRACKET;
        break;
    default:
        nextChar();
        return nextToken();
    }
    nextChar();
    return t;
}