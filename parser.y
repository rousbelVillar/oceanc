%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"    /* defines ValueList and is included by your scanner */

extern int yylex(void);
void yyerror(const char *s);

/* ---- integer table ---- */
typedef struct {
    char* name;
    int value;
    int defined;
} IntEntry;

#define MAX_INTS 256
static IntEntry ints[MAX_INTS];
static int ints_count = 0;

IntEntry* find_int(const char* name) {
    for (int i = 0; i < ints_count; ++i)
        if (strcmp(ints[i].name, name) == 0) return &ints[i];
    return NULL;
}
void define_int(const char* name, int value) {
    IntEntry* e = find_int(name);
    if (e) { e->value = value; e->defined = 1; return; }
    ints[ints_count].name = strdup(name);
    ints[ints_count].value = value;
    ints[ints_count].defined = 1;
    ints_count++;
}

/* ---- string table ---- */
typedef struct {
    char* name;
    char* value;
    int is_const;
} StringEntry;

#define MAX_STRINGS 256
static StringEntry strings[MAX_STRINGS];
static int strings_count = 0;

StringEntry* find_string(const char* name) {
    for (int i = 0; i < strings_count; ++i)
        if (strcmp(strings[i].name, name) == 0) return &strings[i];
    return NULL;
}
void define_string(const char* name, const char* val, int is_const) {
    StringEntry* e = find_string(name);
    if (e) {
        if (e->is_const) {
            fprintf(stderr, "Error: cannot modify const string '%s'\n", name);
            return;
        }
        free(e->value);
        e->value = strdup(val);
        return;
    }
    strings[strings_count].name = strdup(name);
    strings[strings_count].value = strdup(val);
    strings[strings_count].is_const = is_const;
    strings_count++;
}

/* unescape helper for lexer STRING (which includes quotes) */
char* unescape(const char* s) {
    if (!s) return strdup("");
    size_t len = strlen(s);
    if (len < 2) return strdup("");
    const char* p = s + 1; /* skip leading " */
    char* out = malloc(len); /* safe upper bound */
    char* q = out;
    while (*p && p[1]) { /* stop before trailing " */
        if (*p == '\\') {
            p++;
            if (*p == 'n') *q++ = '\n';
            else if (*p == 't') *q++ = '\t';
            else *q++ = *p;
        } else {
            *q++ = *p;
        }
        p++;
    }
    *q = '\0';
    return out;
}

/* SELECT/CASE runtime flags (simple approach used across earlier iterations) */
int select_value = 0;
int case_matched = 0;
int inside_select = 0;
int current_case_active = 0;

/* vlist helpers (ValueList defined in ast.h) */
ValueList* vlist_append(ValueList* head, int v) {
    ValueList* node = malloc(sizeof(ValueList));
    node->value = v;
    node->next = NULL;
    if (!head) return node;
    ValueList* t = head;
    while (t->next) t = t->next;
    t->next = node;
    return head;
}
void free_vlist(ValueList* v) {
    while (v) {
        ValueList* nx = v->next;
        free(v);
        v = nx;
    }
}
%}

/* ----- union and tokens (must match your lexer) ----- */
%union {
    char* sval;
    int   ival;
    ValueList* vlist;
}

/* tokens from scanner.l */
%token CONST INPUT PRINT
%token INTKW STRINGKW
%token SELECTKW CASEKW DEFAULTKW ENDSELECTKW
%token MIDFUNC MOD

%token <ival> NUMBER
%token <sval> STRING   /* lexer returns STRING for string-literal */
%token <sval> ID       /* identifier (integers) */
%token <sval> IDSTR    /* identifier that ends with $ (strings) */

%left '+' '-'
%left '*' '/'
%left MOD

/* nonterminal types */
%type <ival> expr
%type <sval> strexpr
%type <vlist> case_value_list

%%

program:
      program statement
    | /* empty */
    ;

/* ---------------------- STATEMENTS ---------------------- */
statement:

    /* INPUT "prompt", ID; */
    INPUT STRING ',' ID ';'
    {
        if (!inside_select || current_case_active) {
            char* t = unescape($2);
            printf("%s", t);
            free(t);
            int val;
            if (scanf("%d", &val) == 1) define_int($4, val);
            else fprintf(stderr, "Error: failed to read integer for %s\n", $4);
        }
    }

  /* PRINT with a flexible argument list */
  | PRINT print_arg_list ';'
    { /* printing is done inside print_arg_list actions */ }

  /* CONST IDSTR = "text"; */
  | CONST IDSTR '=' STRING ';'
    {
        char* t = unescape($4);
        if (!inside_select || current_case_active) define_string($2, t, 1);
        free(t);
    }

  /* string assignment: IDSTR = strexpr; */
  | IDSTR '=' strexpr ';'
    {
        if (!inside_select || current_case_active) define_string($1, $3, 0);
        free($3);
    }

  /* INTEGER declarations */
  | INTKW ID ';'
    {
        if (!inside_select || current_case_active) define_int($2, 0);
    }

  | INTKW ID '=' expr ';'
    {
        if (!inside_select || current_case_active) define_int($2, $4);
    }

  /* integer assignment */
  | ID '=' expr ';'
    {
        if (!inside_select || current_case_active) define_int($1, $3);
    }

  /* SELECT expr case_blocks ENDSELECT; */
  | SELECTKW expr case_blocks ENDSELECTKW ';'
    {
        inside_select = 0;
        current_case_active = 0;
        case_matched = 0;
    }
  ;

/* ---------- flexible PRINT arguments ---------- */
/* print_arg_list performs the printing as a side effect */
print_arg_list:
      print_arg
    | print_arg_list ',' print_arg
    ;

/* each print_arg prints itself when active */
print_arg:
      STRING
      {
          if (!inside_select || current_case_active) {
              char* t = unescape($1);
              printf("%s", t);
              free(t);
          }
      }
    | IDSTR
      {
          if (!inside_select || current_case_active) {
              StringEntry* s = find_string($1);
              if (!s) fprintf(stderr, "Error: undefined string '%s'\n", $1);
              else printf("%s", s->value);
          }
      }
    | expr
      {
          if (!inside_select || current_case_active) {
              printf("%d", $1);
          }
      }
    ;

/* ---------------- CASE BLOCKS ---------------- */
case_blocks:
      case_blocks case_block
    | case_block
    ;

case_block:
      CASEKW case_value_list ':'
        {
            if (!inside_select) {
                inside_select = 1;
                case_matched = 0;
                current_case_active = 0;
            }
            if (!case_matched) {
                ValueList* p = $2;
                int matched = 0;
                while (p) {
                    if (select_value == p->value) { matched = 1; break; }
                    p = p->next;
                }
                if (matched) { current_case_active = 1; case_matched = 1; }
                else current_case_active = 0;
            } else current_case_active = 0;
            free_vlist($2);
        }
        program
        {
            current_case_active = 0;
        }

    | DEFAULTKW ':'
        {
            if (!inside_select) {
                inside_select = 1;
                case_matched = 0;
                current_case_active = 0;
            }
            if (!case_matched) current_case_active = 1;
            else current_case_active = 0;
        }
        program
        {
            current_case_active = 0;
            if (!case_matched) case_matched = 1;
        }
    ;

/* case value list */
case_value_list:
      expr                 { $$ = vlist_append(NULL, $1); }
    | case_value_list ',' expr  { $$ = vlist_append($1, $3); }
    ;

/* ---------------- STRING EXPRESSIONS ---------------- */
strexpr:
      STRING
        { $$ = unescape($1); }

    | IDSTR
        {
            StringEntry* s = find_string($1);
            $$ = (s ? strdup(s->value) : strdup(""));
        }

    /* allow numeric expressions inside string expressions by converting to string */
    | expr
        {
            /* convert integer to string */
            char buf[64];
            snprintf(buf, sizeof(buf), "%d", $1);
            $$ = strdup(buf);
        }

    | strexpr '+' strexpr
        {
            char* out = malloc(strlen($1) + strlen($3) + 1);
            strcpy(out, $1); strcat(out, $3);
            free($1); free($3);
            $$ = out;
        }

    | MIDFUNC '(' strexpr ',' expr ',' expr ')'
        {
            char* src = $3;
            int start = $5;
            int len   = $7;
            int srclen = (int)strlen(src);
            if (start < 0) start = 0;
            if (len < 0) len = 0;
            if (start >= srclen) $$ = strdup("");
            else {
                int take = (len < srclen - start) ? len : srclen - start;
                char* out = malloc(take + 1);
                memcpy(out, src + start, take);
                out[take] = 0;
                $$ = out;
            }
            free(src);
        }
    ;

/* ---------------- INTEGER EXPRESSIONS ---------------- */
expr:
      NUMBER                     { $$ = $1; }
    | ID                         {
                                    IntEntry* e = find_int($1);
                                    $$ = (e && e->defined) ? e->value : 0;
                                  }
    | expr '+' expr              { $$ = $1 + $3; }
    | expr '-' expr              { $$ = $1 - $3; }
    | expr '*' expr              { $$ = $1 * $3; }
    | expr '/' expr              { $$ = ($3 == 0 ? 0 : $1 / $3); }
    | expr MOD expr              { $$ = ($3 == 0 ? 0 : $1 % $3); }
    | '(' expr ')'               { $$ = $2; }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error: %s\n", s);
}
