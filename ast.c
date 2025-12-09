#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "ast.h"

/* ============================================================
   Esta seccion se encarga de majar los valores de las variables
   ============================================================ */

static Value make_num_val(long v) {
    Value x; x.is_str = 0; x.num = v; x.str = NULL; return x;
}

static Value make_str_val_dup(const char *s) {
    Value x; x.is_str = 1; x.num = 0;
    x.str = s ? strdup(s) : strdup("");
    return x;
}

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
   almacenamiento de variables simples
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

    /* asignamiento de constantes */
    if (var_is_const[c]) {
        fprintf(stderr, "Error: No se puede asignar la constante '%c'\n", name[0]);
        free_val(&v);
        return;
    }

    /* Liberar variables de string */
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
    free_val(&v); /* consumir el valor */
}

/* ============================================================
   Constructores AST
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
    n->sval = strdup(name);
    n->left = a1;
    n->right = a2;
    n->next = a3;
    return n;
}


ASTNode *make_case(ASTNode *match, ASTNode *body, ASTNode *next) {
    ASTNode *n = malloc(sizeof(ASTNode));
    n->type = NODE_CASE;
    n->left  = match;
    n->right = body;
    n->next  = next;
    return n;
}

ASTNode *make_default(ASTNode *body) {
    ASTNode *n = malloc(sizeof(ASTNode));
    n->type = NODE_DEFAULT;
    n->left  = NULL;
    n->right = body;
    n->next  = NULL;
    return n;
}

ASTNode *make_select(ASTNode *expr, ASTNode *cases) {
    ASTNode *n = new_node(NODE_SELECT);
    n->left = expr;
    n->right = cases;
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

     case NODE_SELECT: {
        Value sel = eval(n->left);
        ASTNode *caseptr = n->right;
        ASTNode *defcase = NULL;

        while (caseptr) {
            if (caseptr->type == NODE_CASE) {
                /* evaluar case para que coincida con el valor */
                Value v = eval(caseptr->left);

                    /* igualdad de los valores numericos*/
                if (!v.is_str && !sel.is_str && (v.num == sel.num)) {
                    /* ejecutar el cuerpo del case */
                    exec(caseptr->right);

                    free_val(&v);
                    free_val(&sel);
                    return make_num_val(0);
                }

                free_val(&v);
            }
            else if (caseptr->type == NODE_DEFAULT) {
            defcase = caseptr;
            }
            caseptr = caseptr->next;
        }

        if (defcase) exec(defcase->right);

        free_val(&sel);
        return make_num_val(0);
     }


    case NODE_FUNCALL: {

    if (strcmp(n->sval, "input") == 0) {

        char buffer[256];
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            fprintf(stderr, "Aviso: input no recibido.\n");
            return make_num_val(0);
        }

        // eliminar newline
        buffer[strcspn(buffer, "\n")] = 0;

        // validar que es un número
        char *endptr;
        long val = strtol(buffer, &endptr, 10);

        if (*endptr != '\0') {
            fprintf(stderr, "Aviso: input '%s' no es numerico. usando 0.\n", buffer);
            return make_num_val(0);
        }

            return make_num_val(val);
        }
    if (strcmp(n->sval, "mid") == 0) {

            Value s = eval(n->left);
            Value startV = eval(n->right);

            ASTNode *a3 = n->next;
            Value lenV = a3 ? eval(a3) : make_num_val(-1);

            if (!s.is_str) {
                free_val(&s);
                free_val(&startV);
                free_val(&lenV);
                return make_str_val_dup("");
            }

            const char *src = s.str;

            long start;
            if (startV.is_str) start = atoi(startV.str);
            else start = startV.num;

            long len;
            if (lenV.is_str) len = atoi(lenV.str);
            else len = lenV.num;

            if (start < 0) start = 0;
            size_t srclen = strlen(src);
            if (start >= (long)srclen) {
                free_val(&s); free_val(&startV); free_val(&lenV);
                return make_str_val_dup("");
            }

            if (len < 0) len = (long)srclen - start;
            if (start + len > (long)srclen) len = (long)srclen - start;

            char *out = malloc((size_t)len + 1);
            if (!out) { perror("malloc"); free_val(&s); free_val(&startV); free_val(&lenV); return make_str_val_dup(""); }
            memcpy(out, src + start, (size_t)len);
            out[len] = '\0';

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

        /* operaciones numericas */
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

     case NODE_SELECT: {
        eval(n);
        return;
    }


    default:
        fprintf(stderr, "exec: tipo de nodo desconocido %d\n", n->type);
        return;
    }
}
