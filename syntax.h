#ifndef SYNTAX_TREE
#define SYNTAX_TREE
#define MAXLEN 256
typedef enum 
{
    UNKNOWN, 
    END, 
    INT, 
    ID,  
    MULDIV,
    ADDSUB,
    AND_XOR_OR, 
    ASSIGN,
    LPAREN, 
    RPAREN
} TokenSet;

extern int match (TokenSet token);
extern void Next(void);
extern char* getLexeme(void);

#endif