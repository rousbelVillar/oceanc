#include <stdio.h>
#include "ast.h"

extern FILE *yyin;
int yyparse();
extern ASTNode *program_root;

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("Error opening file");
            return 1;
        }
    }

    yyparse();

    exec(program_root);

    return 0;
}
