%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

extern int yylex();
void yyerror(const char *s);

ASTNode *program_root = NULL;
%}


%union {
    int ival;
    char *sval;
    ASTNode *node;
}


%token DO WHILE PRINT
%token LBRACE RBRACE LPAREN RPAREN SEMI
%token ASSIGN PLUS MINUS
%token <ival> NUMBER
%token <sval> IDENT
%token <sval> STRINGCONST
%token LT GT
%token VAR
%token MUL DIV
%token MOD
%token CONST
%token COMMA
%token ID STRING_LITERAL INT_LITERAL



%left LT GT
%left PLUS MINUS
%left MUL DIV MOD

%type <node> program stmt stmtlist expr

%%

program:
      stmtlist           { program_root = $1; }
    ;

stmtlist:
      stmtlist stmt      { $$ = make_block($1, $2); }
    | stmt               { $$ = make_block(NULL, $1); }
    ;

stmt:
      CONST IDENT ASSIGN expr SEMI
          { $$ = make_constdecl($2, $4); }
    |
      IDENT ASSIGN expr SEMI
          { $$ = make_assign($1, $3); }
    |
      VAR IDENT ASSIGN expr SEMI
          { $$ = make_vardecl($2, $4); }
    |
      PRINT expr SEMI
          { $$ = make_print($2); }
    |
      DO LBRACE stmtlist RBRACE WHILE LPAREN expr RPAREN SEMI
          { $$ = make_dowhile($3, $7); }

    ;

expr:
      NUMBER                 { $$ = make_number($1); }
    | IDENT                  { $$ = make_var($1); }
    | STRINGCONST            { $$ = make_string($1); }
    | LPAREN expr RPAREN     { $$ = $2; }

    /* Function calls allowed inside expressions */
    | IDENT LPAREN expr RPAREN
          { $$ = make_funcall($1, $3, NULL, NULL); }

    | IDENT LPAREN expr COMMA expr RPAREN
          { $$ = make_funcall($1, $3, $5, NULL); }

    | IDENT LPAREN expr COMMA expr COMMA expr RPAREN
          { $$ = make_funcall($1, $3, $5, $7); }

    | expr PLUS expr         { $$ = make_binop('+', $1, $3); }
    | expr MINUS expr        { $$ = make_binop('-', $1, $3); }
    | expr MUL expr          { $$ = make_binop('*', $1, $3); }
    | expr DIV expr          { $$ = make_binop('/', $1, $3); }
    | expr MOD expr          { $$ = make_binop('%', $1, $3); }
    | expr LT expr           { $$ = make_binop('<', $1, $3); }
    | expr GT expr           { $$ = make_binop('>', $1, $3); }
    ;




%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}
