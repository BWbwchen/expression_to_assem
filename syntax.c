#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "syntax.h"

static TokenSet getToken(void);
static TokenSet lookahead = BEGIN;
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
        
        if (c == '&') return AND;
        else if (c == '^') return XOR;
        else if (c == '|') return OR;
    
        //return AND_XOR_OR;    
    } else if (c == '\n' || c == '\0') {
        lexeme[0] = '\0';
        //printf("END\n");
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
    } else if (isalpha(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isalpha(c)) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    } else if (c == EOF) {
        //printf("EOF\n");
        return END_OF_INPUT;
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
    if (lookahead == BEGIN) Next();
    return token == lookahead;
}

char* getLexeme(void)
{
    return lexeme;
}