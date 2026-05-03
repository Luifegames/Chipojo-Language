#include "parser.h"
#include "lexer.h"

Token current_token;

void forward() { current_token = nextToken(); }
void consume(TypeToken type, char *message)
{
    if (current_token.type == type)
        forward();
    else
    {
        printf("%s (wait %d, got %d)\n", message, type, current_token.type);
        exit(1);
    }
}


void assignation(){
    char name[64];
    strcpy(name, current_token.name);
    printf("Assignation '%d'", current_token.type);
    forward();
}


void print_stmt()
{
    consume(TOKEN_PRINT, "not found 'print'");
    consume(TOKEN_LEFTPARENT, "not found '('");
    if (current_token.type == TOKEN_STRING || current_token.type == TOKEN_ID)
    {
        printf("%s \n", current_token.name);
    }else if (current_token.type == TOKEN_NUM)
        {
            printf("%d \n",current_token.value);
        }
        
        forward();
    consume(TOKEN_RIGHTPARENT, "no found')'");
}




void program(){
    while (current_token.type != TOKEN_EOF)
    {   
        if (current_token.type == TOKEN_PRINT)
        {
            print_stmt();
        }
        else if (current_token.type == TOKEN_ID)
        {
            assignation();
        }
        else
        {
            printf("Error, invalid statement (token %d)\n", current_token.type);
            exit(1);
        }
    }
}