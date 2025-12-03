#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ===========================================================
   NODE CONSTRUCTORS
   =========================================================== */

static ASTNode *new_node(NodeType type) {
    ASTNode *n = malloc(sizeof(ASTNode));
    n->type = type;
    n->ival = 0;
    n->sval = NULL;
    n->left = NULL;
    n->right = NULL;
    return n;
}

ASTNode *make_number(int v) {
    ASTNode *n = new_node(NODE_NUMBER);
    n->ival = v;
    return n;
}

ASTNode *make_var(char *name) {
    ASTNode *n = new_node(NODE_VAR);
    n->sval = strdup(name);
    return n;
}

ASTNode *make_binop(int op, ASTNode *l, ASTNode *r) {
    ASTNode *n = new_node(NODE_BINOP);
    n->ival = op;
    n->left = l;
    n->right = r;
    return n;
}

ASTNode *make_block(ASTNode *a, ASTNode *b) {
    ASTNode *n = new_node(NODE_BLOCK);
    n->left = a;
    n->right = b;
    return n;
}

ASTNode *make_assign(char *name, ASTNode *expr) {
    ASTNode *n = new_node(NODE_ASSIGN);
    n->sval = strdup(name);
    n->left = expr;
    return n;
}

ASTNode *make_print(ASTNode *expr) {
    ASTNode *n = new_node(NODE_PRINT);
    n->left = expr;
    return n;
}

ASTNode *make_dowhile(ASTNode *body, ASTNode *cond) {
    ASTNode *n = new_node(NODE_DOWHILE);
    n->left = body;
    n->right = cond;
    return n;
}

ASTNode *make_vardecl(char *name, ASTNode *expr) {
    ASTNode *n = new_node(NODE_VARDECL);
    n->sval = strdup(name);
    n->left = expr;
    return n;
}

ASTNode *make_string(char *s) {
    ASTNode *n = new_node(NODE_STRING);
    n->sval = strdup(s);
    return n;
}



/* ===========================================================
   SIMPLE VARIABLE ENVIRONMENT (1-char names)
   =========================================================== */

static int vars[256];
static int defined[256];

int get_var(const char *name) {
    unsigned c = (unsigned)name[0];
    if (!defined[c]) {
        fprintf(stderr, "Error: variable '%s' not defined\n", name);
        return 0;
    }
    return vars[c];
}

void set_var(const char *name, int value) {
    unsigned c = (unsigned)name[0];
    vars[c] = value;
    defined[c] = 1;
}

/* ===========================================================
   EXPRESSION EVALUATION
   =========================================================== */

int eval(ASTNode *n) {
    if (!n) return 0;

    switch (n->type) {

        case NODE_NUMBER:
            return n->ival;

        case NODE_VAR:
            return get_var(n->sval);
        case NODE_STRING:
            /* return strings via pointer value stored in sval */
            return (long)n->sval;


        case NODE_BINOP: {
            int L = eval(n->left);
            int R = eval(n->right);

            switch (n->ival) {
                case '+': return L + R;
                case '-': return L - R;
                case '*': return L * R;
                case '/': return R != 0 ? L / R : 0;

                case '<': return L < R;
                case '>': return L > R;
                case '%': return L % R;


                default:
                    fprintf(stderr, "Unknown operator '%c'\n", n->ival);
                    return 0;
            }
        }

        default:
            fprintf(stderr, "Invalid expression node\n");
            return 0;
    }
}

/* ===========================================================
   STATEMENT EXECUTION
   =========================================================== */

void exec(ASTNode *n) {
    if (!n) return;

    switch (n->type) {

        case NODE_BLOCK:
            exec(n->left);
            exec(n->right);
            return;

        case NODE_ASSIGN:
            set_var(n->sval, eval(n->left));
            return;

        case NODE_VARDECL:
            set_var(n->sval, eval(n->left));
            return;

        case NODE_PRINT: {
            long v = eval(n->left);
            if (n->left->type == NODE_STRING)
                printf("%s\n", (char*)v);
            else
                printf("%ld\n", v);
            return;
        }
        case NODE_DOWHILE:
            do {
                exec(n->left);
            } while (eval(n->right));
            return;

        default:
            fprintf(stderr, "Unknown statement node\n");
            return;
    }
}
