/* ast.c
   Complete AST implementation:
   - constructor functions (make_*)
   - Value system (string + number)
   - eval() and exec()
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "ast.h"

/* ============================================================
   Value helpers
   ============================================================ */

static Value make_num_val(long v) {
    Value x; x.is_str = 0; x.num = v; x.str = NULL; return x;
}

static Value make_str_val_dup(const char *s) {
    Value x; x.is_str = 1; x.num = 0;
    x.str = s ? strdup(s) : strdup("");
    return x;
}

/* take ownership of malloced s */
static Value make_str_val_take(char *s) {
    Value x; x.is_str = 1; x.num = 0;
    x.str = s ? s : strdup("");
    return x;
}

static void free_val(Value *v) {
    if (!v) return;
    if (v->is_str && v->str) free(v->str);
    v->is_str = 0;
    v->str = NULL;
    v->num = 0;
}

/* ============================================================
   Simple variable store (1-char names)
   ============================================================ */

static int   var_defined[256] = {0};
static int   var_is_str[256] = {0};
static long  var_num[256]    = {0};
static char *var_str[256]    = {0};
static int var_is_const[256] = {0};


static Value get_var_value(const char *name) {
    if (!name || !name[0]) return make_num_val(0);
    unsigned c = (unsigned char)name[0];

    if (!var_defined[c]) return make_num_val(0);

    if (var_is_str[c])
        return make_str_val_dup(var_str[c] ? var_str[c] : "");
    else
        return make_num_val(var_num[c]);
}

static void set_var_value(const char *name, Value v) {
    if (!name || !name[0]) { free_val(&v); return; }

    unsigned c = (unsigned char)name[0];

    /* block assignment to constants */
    if (var_is_const[c]) {
        fprintf(stderr, "Error: cannot assign to constant '%c'\n", name[0]);
        free_val(&v);
        return;
    }

    /* free old string if present */
    if (var_is_str[c] && var_str[c]) {
        free(var_str[c]);
        var_str[c] = NULL;
    }

    if (v.is_str) {
        var_is_str[c] = 1;
        var_str[c] = strdup(v.str ? v.str : "");
    } else {
        var_is_str[c] = 0;
        var_num[c] = v.num;
    }

    var_defined[c] = 1;
    free_val(&v); /* consume value */
}

/* ============================================================
   AST Constructors (the missing pieces!)
   ============================================================ */

static ASTNode *new_node(NodeType t) {
    ASTNode *n = calloc(1, sizeof(ASTNode));
    n->type = t;
    return n;
}

ASTNode *make_number(int v) {
    ASTNode *n = new_node(NODE_NUMBER);
    n->ival = v;
    return n;
}

ASTNode *make_string(char *s) {
    ASTNode *n = new_node(NODE_STRING);
    n->sval = strdup(s);
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

ASTNode *make_assign(char *name, ASTNode *expr) {
    ASTNode *n = new_node(NODE_ASSIGN);
    n->sval = strdup(name);
    n->left = expr;
    return n;
}

ASTNode *make_vardecl(char *name, ASTNode *expr) {
    ASTNode *n = new_node(NODE_VARDECL);
    n->sval = strdup(name);
    n->left = expr;
    return n;
}

ASTNode *make_print(ASTNode *expr) {
    ASTNode *n = new_node(NODE_PRINT);
    n->left = expr;
    return n;
}

ASTNode *make_block(ASTNode *a, ASTNode *b) {
    ASTNode *n = new_node(NODE_BLOCK);
    n->left = a;
    n->right = b;
    return n;
}

ASTNode *make_dowhile(ASTNode *body, ASTNode *cond) {
    ASTNode *n = new_node(NODE_DOWHILE);
    n->left = body;
    n->right = cond;
    return n;
}
ASTNode *make_constdecl(char *name, ASTNode *expr) {
    ASTNode *n = new_node(NODE_CONSTDECL);
    n->sval = strdup(name);
    n->left = expr;
    return n;
}
ASTNode *make_funcall(char *name, ASTNode *a1, ASTNode *a2, ASTNode *a3) {
    ASTNode *n = new_node(NODE_FUNCALL);
    n->sval = strdup(name);   // function name ("mid")
    n->left = a1;             // first argument
    n->right = a2;            // second argument
    n->ival = (intptr_t)a3;   // store third arg somewhere
    return n;
}

/* ============================================================
   eval()
   ============================================================ */

Value eval(ASTNode *n) {
    if (!n) return make_num_val(0);

    Value a, b;

    switch (n->type) {

    case NODE_NUMBER:
        return make_num_val(n->ival);

    case NODE_STRING:
        return make_str_val_dup(n->sval);

    case NODE_VAR:
        return get_var_value(n->sval);

    case NODE_FUNCALL: {
    if (strcmp(n->sval, "mid") == 0) {

        Value s = eval(n->left);
        Value startV = eval(n->right);
        ASTNode *a3 = (ASTNode*)n->ival;
        Value lenV = a3 ? eval(a3) : make_num_val(-1);

        if (!s.is_str) {
            free_val(&s);
            free_val(&startV);
            free_val(&lenV);
            return make_str_val_dup("");   // mid on non-string yields ""
        }

        const char *src = s.str;
        long start = startV.num;  // 0-based
        long len = lenV.num;

        if (start < 0) start = 0;
        if (start >= strlen(src)) {
            free_val(&s); free_val(&startV); free_val(&lenV);
            return make_str_val_dup("");
        }

        if (len < 0) len = strlen(src) - start;

        if (start + len > strlen(src))
            len = strlen(src) - start;

        char *out = malloc(len + 1);
        memcpy(out, src + start, len);
        out[len] = 0;

        free_val(&s);
        free_val(&startV);
        free_val(&lenV);

        return make_str_val_take(out);
    }

        fprintf(stderr, "Unknown function: %s\n", n->sval);
        return make_num_val(0);
    }


    case NODE_BINOP: {
        a = eval(n->left);
        b = eval(n->right);
        int op = n->ival;

        if (op == '+') {
            if (a.is_str || b.is_str) {
                char bufA[64], bufB[64];
                const char *sa = a.is_str ? a.str : (snprintf(bufA, 64, "%ld", a.num), bufA);
                const char *sb = b.is_str ? b.str : (snprintf(bufB, 64, "%ld", b.num), bufB);

                size_t len = strlen(sa) + strlen(sb) + 1;
                char *out = malloc(len);
                strcpy(out, sa);
                strcat(out, sb);

                free_val(&a); free_val(&b);
                return make_str_val_take(out);
            }

            long res = a.num + b.num;
            free_val(&a); free_val(&b);
            return make_num_val(res);
        }

        /* numeric operations */
        long L = a.is_str ? atoi(a.str) : a.num;
        long R = b.is_str ? atoi(b.str) : b.num;
        long r = 0;

        switch (op) {
            case '-': r = L - R; break;
            case '*': r = L * R; break;
            case '/': r = (R == 0 ? 0 : L / R); break;
            case '%': r = (R == 0 ? 0 : L % R); break;
            case '<': r = (L < R); break;
            case '>': r = (L > R); break;
        }

        free_val(&a); free_val(&b);
        return make_num_val(r);
    }

    default:
        fprintf(stderr, "eval: unsupported node type %d\n", n->type);
        return make_num_val(0);
    }
}

/* ============================================================
   exec()
   ============================================================ */

void exec(ASTNode *n) {
    if (!n) return;

    switch (n->type) {

    case NODE_BLOCK:
        exec(n->left);
        exec(n->right);
        return;

    case NODE_ASSIGN: {
        Value v = eval(n->left);
        set_var_value(n->sval, v);
        return;
    }

    case NODE_VARDECL: {
        Value v = eval(n->left);
        set_var_value(n->sval, v);
        return;
    }

  case NODE_PRINT: {
    Value v = eval(n->left);

    if (v.is_str)
        printf("%s\n", v.str);
    else
        printf("%ld\n", v.num);

    free_val(&v);
    return;
}



    case NODE_DOWHILE: {
        do {
            exec(n->left);
            Value cond = eval(n->right);
            int truth = cond.is_str ? (cond.str && cond.str[0]) : (cond.num != 0);
            free_val(&cond);
            if (!truth) break;
        } while (1);
        return;
    }

    case NODE_CONSTDECL: {
       char name = n->sval[0];

        if (var_defined[(unsigned char)name]) {
            fprintf(stderr, "Error: cannot redeclare '%c'\n", name);
            return;
        }

        Value v = eval(n->left);

        var_is_const[(unsigned char)name] = 1;

        set_var_value(n->sval, v);
        return;
    }

    default:
        fprintf(stderr, "exec: unknown node type %d\n", n->type);
        return;
    }
}
