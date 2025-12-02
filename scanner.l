%{
#include "ast.h"
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>

int yywrap(void) { return 1; }

static void dbg_tok(const char *tokname, const char *yytext) {
    if (!tokname) tokname = "(char)";
    if (yytext) fprintf(stderr, "LEXER -> %s : '%s'\n", tokname, yytext);
    else fprintf(stderr, "LEXER -> %s\n", tokname);
}
%}

STRING_LITERAL    \"([^\\\"]|\\.)*\"

%%

"CONST"        { dbg_tok("CONST", yytext); return CONST; }
"INPUT"        { dbg_tok("INPUT", yytext); return INPUT; }
"PRINT"        { dbg_tok("PRINT", yytext); return PRINT; }

"INT"          { dbg_tok("INTKW", yytext); return INTKW; }
"STRING"       { dbg_tok("STRINGKW", yytext); return STRINGKW; }

"SELECT"       { dbg_tok("SELECTKW", yytext); return SELECTKW; }
"CASE"         { dbg_tok("CASEKW", yytext); return CASEKW; }
"DEFAULT"      { dbg_tok("DEFAULTKW", yytext); return DEFAULTKW; }
"ENDSELECT"    { dbg_tok("ENDSELECTKW", yytext); return ENDSELECTKW; }

"MOD"          { dbg_tok("MOD", yytext); return MOD; }

"MID$"         { dbg_tok("MIDFUNC", yytext); return MIDFUNC; }

[a-zA-Z_][a-zA-Z0-9_]*\$ {
    dbg_tok("IDSTR", yytext);
    yylval.sval = strdup(yytext);
    return IDSTR;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    dbg_tok("ID", yytext);
    yylval.sval = strdup(yytext);
    return ID;
}

{STRING_LITERAL} {
    dbg_tok("STRING", yytext);
    yylval.sval = strdup(yytext);
    return STRING;
}

[0-9]+ {
    dbg_tok("NUMBER", yytext);
    yylval.ival = atoi(yytext);
    return NUMBER;
}

","     { dbg_tok(",", yytext); return ','; }
";"     { dbg_tok(";", yytext); return ';'; }
":"     { dbg_tok(":", yytext); return ':'; }
"("     { dbg_tok("(", yytext); return '('; }
")"     { dbg_tok(")", yytext); return ')'; }
"="     { dbg_tok("=", yytext); return '='; }
"+"     { dbg_tok("+", yytext); return '+'; }
"-"     { dbg_tok("-", yytext); return '-'; }
"*"     { dbg_tok("*", yytext); return '*'; }
"/"     { dbg_tok("/", yytext); return '/'; }

[ \t\r\n]+    ;  /* ignore whitespace */

.   { /* unknown char — log it to help debugging */
        char cstr[2] = { yytext[0], '\0' };
        dbg_tok("UNKNOWN_CHAR", cstr);
    }
%%

