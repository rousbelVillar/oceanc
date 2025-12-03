static void dbg_tok(const char *tokname, const char *yytext) {
    if (!tokname) tokname = "(char)";
    if (yytext) fprintf(stderr, "LEXER -> %s : '%s'\n", tokname, yytext);
    else fprintf(stderr, "LEXER -> %s\n", tokname);
}

    if (!tokname) tokname = "(char)";
    if (yytext) fprintf(stderr, "LEXER -> %s : '%s'\n", tokname, yytext);
    else fprintf(stderr, "LEXER -> %s\n", tokname);