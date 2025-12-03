%{
#include "ast.h"
#include "parser.tab.h"
%}

%%


[ \t\n\r]+          ;
"do"                { return DO; }
"while"             { return WHILE; }
"print"             { return PRINT; }

"{"                 { return LBRACE; }
"}"                 { return RBRACE; }
"("                 { return LPAREN; }
")"                 { return RPAREN; }
";"                 { return SEMI; }
"="                 { return ASSIGN; }
"+"                 { return PLUS; }
"-"                 { return MINUS; }
"<"                 { return LT; }
">"                 { return GT; }
"var"               { return VAR; }
"*"                 { return MUL; }
"/"                 { return DIV; }
"%"                 { return MOD; }





[0-9]+              { yylval.ival = atoi(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.sval = strdup(yytext); return IDENT; }
\"([^\\\"]|\\.)*\"    {
                        yylval.sval = strdup(yytext + 1);
                        yylval.sval[strlen(yylval.sval) - 1] = 0;
                        return STRINGCONST;
                      }



.                   ;

%%

int yywrap() { return 1; }
static void dbg_tok(const char *tok, const char *txt) {
    fprintf(stderr, "LEXER -> %s : '%s'\n", tok, txt ? txt : "");
}

