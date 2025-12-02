#ifndef AST_H
#define AST_H

typedef struct ValueList {
    int value;
    struct ValueList* next;
} ValueList;

/* prototype for the helper implemented in parser.y */
ValueList* vlist_append(ValueList* head, int v);

#endif /* AST_H */
