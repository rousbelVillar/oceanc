#ifndef AST_H
#define AST_H

typedef enum {
    NODE_NUMBER,
    NODE_STRING,
    NODE_VAR,
    NODE_BINOP,
    NODE_ASSIGN,
    NODE_PRINT,
    NODE_BLOCK,
    NODE_DOWHILE,
    NODE_VARDECL
} NodeType;


typedef struct ASTNode {
    NodeType type;
    int ival;
    char *sval;
    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

/* Constructors */
ASTNode *make_number(int v);
ASTNode *make_var(char *name);
ASTNode *make_binop(int op, ASTNode *l, ASTNode *r);
ASTNode *make_block(ASTNode *first, ASTNode *second);
ASTNode *make_assign(char *name, ASTNode *expr);
ASTNode *make_print(ASTNode *expr);
ASTNode *make_dowhile(ASTNode *body, ASTNode *cond);
ASTNode *make_vardecl(char *name, ASTNode *expr);
ASTNode *make_string(char *s);




/* Interpreter functions */
int eval(ASTNode *n);
void exec(ASTNode *n);

#endif
