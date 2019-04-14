#define DEBUG
#define TBLSIZE 64
#define TOTALSTATEMENT 50
#define OK 100
#define NO 99
#define LEFT 5
#define RIGHT 6
#define ROOT 8

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


int ASSIGN_NUMBER = 0;
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

typedef enum {MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NAN, PRESENT_ERROR} ErrorType;
void error(ErrorType errorNum);

void Make_assem (BTNode* root, int type, int *id);
typedef struct List
{
	int data;
	int can_use;
}Reg;
Reg reg[8];
int OK_register (void);
int ID_index(char name[]);



int main ()
{
	#ifdef DEBUG
	freopen ("text.in", "r", stdin);
	#endif
	//initial 
	//make x, y, z space
	for (int i = 0; i < 3; ++i) {
		if (i == 0) strcpy(table[i].name, "x");
		else if (i == 1) strcpy(table[i].name, "y");
		else if (i == 2) strcpy(table[i].name, "z");
		table[i].val = 0;
		//printf("initial ; %s %d", table[i].name, table[i].val);
	}
	//initial reg
	for (int i = 0; i < 7; ++i) {
		reg[i].data = 0;
		reg[i].can_use = OK;
	}

    while (!match(END_OF_INPUT)) {
        //printf(">> ");
        statement();
		ASSIGN_NUMBER = 0;
		total_root++;
    }
	//printf("success parsing! %d\n", total_root);
	for (int i = 0; i < total_root; ++i) {
		Make_assem(root_set[i], ROOT, NULL);
		freeTree(root_set[i]);
	}
	printf("MOV r0 [0]\nMOV r1 [4]\nMOV r2 [8]\n");
	printf("EXIT 0\n");
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
	while (match(OR)) { // tail recursion => while
		retp = makeNode(OR, getLexeme());
		Next();
		retp->right = expr();
		retp->left = left;
		left = retp;
	}
	while (match(XOR)) { // tail recursion => while
		retp = makeNode(XOR, getLexeme());
		Next();
		retp->right = expr();
		retp->left = left;
		left = retp;
	}
	while (match(AND)) { // tail recursion => while
		retp = makeNode(AND, getLexeme());
		Next();
		retp->right = expr();
		retp->left = left;
		left = retp;
	}
	if (match(UNKNOWN)) error(PRESENT_ERROR);
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
	if (match(UNKNOWN)) error(PRESENT_ERROR);
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
	if (match(UNKNOWN)) error(PRESENT_ERROR);
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
		//if (left->val == MISS && ASSIGN_NUMBER) error(NOTFOUND);
		strcpy(tmpstr, getLexeme());
		Next();
		ASSIGN_NUMBER++;
		if (match(ASSIGN)) {
			//prevent double assign
			if (ASSIGN_NUMBER > 1) error(PRESENT_ERROR);
			retp = makeNode(ASSIGN, getLexeme());
			Next();
			retp->right = overall();
			retp->left = left;
		} else {
			//printf("here\n");
			//if (ASSIGN_NUMBER) error(PRESENT_ERROR);
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
		retp = overall();
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
	printf("EXIT 1\n");
	exit(1);
	return;

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
		break;
	case PRESENT_ERROR:
		fprintf(stderr, "Present error\n");
	}
	exit(0);
}
// statement:= END | expr END
void statement (void)
{
	BTNode* retp;
	
	if (match(END)) {
		//printf(">> ");
		Next();
	} else {
		retp = overall();
		if (match(END) || match(END_OF_INPUT)) {
			//printf("%d\n", evaluateTree(retp));
			static int id = 0;
			printPrefix(retp);
			printf("\n");
			root_set[id++] = retp;
			//freeTree(retp);

			//printf(">> ");
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
			if (ASSIGN_NUMBER) error(NOTFOUND);
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


int OK_register (void )
{
	for (int i = 0; i < 8; ++i)
		if (reg[i].can_use == OK) {
			reg[i].can_use = NO;
			return i;
		}
}

int ID_index (char name[])
{
	for (int i = 0; i < sbcount; ++i) {
		if (strcmp(name, table[i].name) == 0) {
			return 4*i;
		}
	}
}

void Make_assem (BTNode *root, int type, int *id)
{
	int left_id = 0, right_id = 0;
	if (root != NULL) {
		switch (root->token) {
		case ID:
			*id = OK_register();
			printf("MOV r%d [%d]\n", *id, ID_index(root->lexeme));
			break;
		case INT:
			*id = OK_register();
			printf("MOV r%d %d\n", *id, root->val);
			break;
		case ASSIGN:
			Make_assem(root->right, RIGHT, &right_id);
			printf ("MOV [%d] r%d\n", ID_index(root->left->lexeme), right_id);
			reg[right_id].can_use = OK;
			break;
		case ADDSUB:
		case MULDIV:
		case AND:
		case OR:
		case XOR:
			Make_assem(root->right, RIGHT, &right_id);
			Make_assem(root->left, LEFT, &left_id);
			if (strcmp(root->lexeme, "+") == 0) {
				printf ("ADD r%d r%d\n", left_id, right_id);
				*id = left_id;
				reg[right_id].can_use = OK;
				break;
			}
			else if (strcmp(root->lexeme, "-") == 0) {
				printf ("SUB r%d r%d\n", left_id, right_id);
				*id = left_id;
				reg[right_id].can_use = OK;
				break;
			}
			else if (strcmp(root->lexeme, "*") == 0) {
				printf ("MUL r%d r%d\n", left_id, right_id);
				*id = left_id;
				reg[right_id].can_use = OK;
				break;
			}
			else if (strcmp(root->lexeme, "/") == 0) {
				printf ("DIV r%d r%d\n", left_id, right_id);
				*id = left_id;
				reg[right_id].can_use = OK;
				break;
			} 
			else if (strcmp(root->lexeme, "&") == 0) {
				printf ("AND r%d r%d\n", left_id, right_id);
				*id = left_id;
				reg[right_id].can_use = OK;
				break;
			} 
			else if (strcmp(root->lexeme, "^") == 0) {
				printf ("XOR r%d r%d\n", left_id, right_id);
				*id = left_id;
				reg[right_id].can_use = OK;
				break;
			} 
			else if (strcmp(root->lexeme, "|") == 0) {
				printf ("OR r%d r%d\n", left_id, right_id);
				*id = left_id;
				reg[right_id].can_use = OK;
				break;
			} 
		default:
			return;
		}
	}
	return ;
}