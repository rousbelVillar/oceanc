%{
#include "ast.h"
#include "parser.tab.h"
%}

%%


[ \t\n\r]+          ;
"do"                { return DO; }
"while"             { return WHILE; }
"print"             { return PRINT; }
"const"             { return CONST; }
"select"            { return SELECT; }
"case"              { return CASE; }
"default"           { return DEFAULT; }
"endselect"         { return ENDSELECT; }


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
","                 { return COMMA; }
":"                 { return COLON; }


[0-9]+              { yylval.ival = atoi(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.sval = strdup(yytext); return IDENT; }
\"([^\\\"]|\\.)*\"  {
    int len = yyleng;

    /* remove surrounding quotes safely */
    char *s = malloc(len - 1);
    memcpy(s, yytext + 1, len - 2);
    s[len - 2] = '\0';

    yylval.sval = s;
    return STRINGCONST;
}

.                   ;

%%

int yywrap() { return 1; }
static void dbg_tok(const char *tok, const char *txt) {
    fprintf(stderr, "LEXER -> %s : '%s'\n", tok, txt ? txt : "");
}

