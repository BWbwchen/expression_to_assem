#define DEBUG
#define TBLSIZE 65535
#define TOTALSTATEMENT 50

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"


typedef struct {
    char name[MAXLEN];
    int val;
} Symbol;

Symbol table[TBLSIZE] ;


int DETECT_ASSIGN = 0;
int sbcount = 3;

typedef struct _Node {
    char lexeme[MAXLEN];
    TokenSet token;
    int val;
    struct _Node *left, *right;
} BTNode;

BTNode* root_set[TOTALSTATEMENT] = {0};
int total_root = 0;

void statement(void);
BTNode* overall(void);
BTNode* expr(void);
BTNode* term(void);
BTNode* factor(void);
BTNode* makeNode(TokenSet tok, const char *lexe);


int getval(void);
int setval(char*, int);

void freeTree(BTNode *root);
void printPrefix(BTNode *root);

typedef enum {MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NAN} ErrorType;
void error(ErrorType errorNum);

// call this function will print the assembly code
// based on the syntax tree
void assemble_tree(BTNode* now_root);



int main ()
{
	#ifdef DEBUG
	freopen ("text.in", "r", stdin);
	#endif
	//make x, y, z space
	for (int i = 0; i < 3; ++i) {
		Symbol temp;
		table[i] = temp;
		if (i == 0) strcpy(table[i].name, "x");
		else if (i == 1) strcpy(table[i].name, "y");
		else if (i == 2) strcpy(table[i].name, "z");
		table[i].val = 0;
		//printf("initial ; %s %d", table[i].name, table[i].val);
	}
    while (!match(END_OF_INPUT)) {
        //printf(">> ");
        statement();
		DETECT_ASSIGN = 0;
    }
	//printf("success parsing!\n");
	//assemble_tree();
    return 0;
}

/* clean a tree */
void freeTree(BTNode *root)
{
	if (root!=NULL) {
		freeTree(root->left);
		freeTree(root->right);
		free(root);
	}
}

/* print a tree by pre-order */
void printPrefix(BTNode *root)
{
	if (root != NULL) {
		printf("%s ", root->lexeme);
		printPrefix(root->left);
		printPrefix(root->right);
	}
}

/* 
	create a node without any child 
	the string in the node is based on lexe 
*/
BTNode* makeNode(TokenSet tok, const char *lexe)
{
	BTNode *node = (BTNode*) malloc(sizeof(BTNode));
	strcpy(node->lexeme, lexe);
	node->token= tok;
	node->val = 0;
	node->left = NULL;
	node->right = NULL;
	return node;
}
// deal of precendence
// need to deal with & > ^ > |
BTNode* overall (void)
{
	BTNode *retp, *left;
	retp = left = expr();
	while (match(AND_XOR_OR)) { // tail recursion => while
		
		retp = makeNode(AND_XOR_OR, getLexeme());
		Next();
		retp->right = expr();
		retp->left = left;
		left = retp;
		
	}
	return retp;

}

//  expr        := term expr_tail
//  expr_tail   := ADD_SUB_AND_OR_XOR term expr_tail | NIL
BTNode* expr(void)
{
	BTNode *retp, *left;
	retp = left = term();
	while (match(ADDSUB)) { // tail recursion => while
		
		retp = makeNode(ADDSUB, getLexeme());
		Next();
		retp->right = term();
		retp->left = left;
		left = retp;
		
	}
	return retp;
}

//  term        := factor term_tail
//  term_tail := MULDIV factor term_tail | NIL
BTNode* term(void)
{
	BTNode *retp, *left;
	retp = left = factor();
	while (match(MULDIV)) { // tail recursion => while
		retp = makeNode(MULDIV, getLexeme());
		Next();
		retp->right = factor();
		retp->left = left;
		left = retp;
	}
	return retp;
}

BTNode* factor(void)
{
	BTNode* retp = NULL;
	char tmpstr[MAXLEN];

	if (match(INT)) {
		retp =  makeNode(INT, getLexeme());
		retp->val = getval();
		Next();
	} else if (match(ID)) {
		BTNode* left = makeNode(ID, getLexeme());
		left->val = getval();
		//if (left->val == MISS && DETECT_ASSIGN) error(NOTFOUND);
		strcpy(tmpstr, getLexeme());
		Next();
		if (match(ASSIGN)) {
			DETECT_ASSIGN = 1;
			retp = makeNode(ASSIGN, getLexeme());
			Next();
			retp->right = expr();
			retp->left = left;
		} else {
			retp = left;
		}
	} else if (match(ADDSUB)) {
		strcpy(tmpstr, getLexeme());
		Next();
		if (match(ID) || match(INT)) {
			retp = makeNode(ADDSUB, tmpstr);
			if (match(ID))
				retp->right = makeNode(ID, getLexeme());
			else
				retp->right = makeNode(INT, getLexeme());
			retp->right->val = getval();
			retp->left = makeNode(INT, "0");
			retp->left->val = 0;
			Next();
		} else {
			error(NOTNUMID);
		}
	} else if (match(LPAREN)) {
		Next();
		retp = expr();
		if (match(RPAREN)) {
			Next();
		} else {
			error(MISPAREN);
		}
	} else {
		error(NOTNUMID);
	}
	return retp;
}

void error(ErrorType errorNum)
{
	switch (errorNum) {
	case MISPAREN:
		fprintf(stderr, "Mismatched parenthesis\n");
		break;
	case NOTNUMID:
		fprintf(stderr, "Number or identifier expected\n");
		break;
	case NOTFOUND:
		fprintf(stderr, "%s not defined\n", getLexeme());
		break;
	case RUNOUT:
		fprintf(stderr, "Out of memory\n");
		break;
	case NAN:
		fprintf(stderr, "Not a number\n");
	}
	exit(0);
}
// statement:= END | expr END
void statement (void)
{
	BTNode* retp;
	
	if (match(END)) {
		printf(">> ");
		Next();
	} else {
		retp = overall();
		if (match(END)) {
			//printf("%d\n", evaluateTree(retp));
			printPrefix(retp);
			printf("\n");
			freeTree(retp);

			printf(">> ");
			Next();
		}
	}
}


int getval(void)
{
    int i, retval, found;

    if (match(INT)) {
        retval = atoi(getLexeme());
    } else if (match(ID)) {
        i = 0; found = 0; retval = 0;
        while (i<sbcount && !found) {
            if (strcmp(getLexeme(), table[i].name)==0) {
                retval = table[i].val;
                found = 1;
                break;
            } else {
                i++;
            }
        }
        if (!found ) {
			if (DETECT_ASSIGN) error(NOTFOUND);
            if (sbcount < TBLSIZE) {
                strcpy(table[sbcount].name, getLexeme());
                table[sbcount].val = 0;
                sbcount++;
            } else {
                error(RUNOUT);
            }
        }
    }
    return retval;
}

int setval(char *str, int val)
{
    int i, retval;
    i = 0;
    while (i<sbcount) {
        if (strcmp(str, table[i].name)==0) {
            table[i].val = val;
            retval = val;
            break;
        } else {
            i++;
        }
    }
    return retval;
}

void assemble_tree(BTNode* now_root)
{

}