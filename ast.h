#include <stdint.h>
#ifndef AST_H
#define AST_H
#define IS_STRING_NODE(n) ((n)->type == NODE_STRING)
typedef struct Value {
    enum {VAL_INT, VAL_STRING} type;
    int is_str;
    long num;
    char *str;
} Value;

typedef enum {
    NODE_NUMBER,
    NODE_STRING,
    NODE_VAR,
    NODE_BINOP,
    NODE_ASSIGN,
    NODE_PRINT,
    NODE_BLOCK,
    NODE_DOWHILE,
    NODE_VARDECL,
    NODE_CONSTDECL,
    NODE_FUNCALL,
    NODE_SELECT,
    NODE_CASE,
    NODE_DEFAULT

} NodeType;

typedef struct ASTNode {
    NodeType type;
    intptr_t ival;
    char *sval;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *next;
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
ASTNode *make_constdecl(char *name, ASTNode *expr);
ASTNode *make_funcall(char *name, ASTNode *arg1, ASTNode *arg2, ASTNode *arg3);
ASTNode *make_select(ASTNode *expr, ASTNode *cases);
ASTNode *make_case(ASTNode *match, ASTNode *body, ASTNode *next);
ASTNode *make_default(ASTNode *body);



Value eval(ASTNode *n);
void exec(ASTNode *n);


#endif
