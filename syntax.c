#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "syntax.h"

static TokenSet getToken(void);
static TokenSet lookahead = UNKNOWN;
static char lexeme[MAXLEN];


TokenSet getToken(void)
{
    int i;
    char c;

    while ( (c = fgetc(stdin)) == ' ' || c== '\t' );  

    if (isdigit(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i<MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ADDSUB;
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '&' || c == '^' || c == '|') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        /*
        if (c == '&') return AND;
        else if (c == '^') return XOR;
        else if (c == '|') return OR;
        */
       return AND_XOR_OR;    
    } else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (isalpha(c) || c == '_') {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isalpha(c) || isdigit(c) || c == '_') {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    } else {
        return UNKNOWN;
    }
}

void Next(void)
{
    lookahead = getToken();
}
//does token match the lookahead ?
int match(TokenSet token)
{
    if (lookahead == UNKNOWN) Next();
    return token == lookahead;
}

char* getLexeme(void)
{
    return lexeme;
}