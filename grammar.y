%{
  #include <stdio.h>
  #include "syntax.h"
  void yyerror(const char* msg) {
     fprintf(stderr, "%s\n", msg);
  }
  int yylex();
  void start(Node root);
%}
%union {
    Node node;
    char *string;
    Number number;
}
%type <node> item_list item function const_var name param_list param_list_ param expr_list expr if expr_op func_call var_ref number
%token KW_LET KW_IF KW_ELSE
%left SYM_OR
%left SYM_AND
%left SYM_EQ SYM_NE
%left SYM_LT SYM_GT SYM_LE SYM_GE
%left SYM_PLUS SYM_MINUS
%left SYM_STAR SYM_SLASH
%left _UMINUS SYM_BANG
%token <string> K_IDENT
%token <number> K_INT K_FLOAT
%%
program     : item_list { start($1); }
            ;
item_list   :                { $$ = Node_new(ITEM_LIST); }
            | item_list item {
                  Node_append_child(&$1, $2);
                  $$ = $1;
              }
            ;
item        : function {
                  $$ = Node_new(ITEM);
                  Node_append_child(&$$, $1);
              }
            | const_var {
                  $$ = Node_new(ITEM);
                  Node_append_child(&$$, $1);
              }
            | expr {
                  $$ = Node_new(ITEM);
                  Node_append_child(&$$, $1);
            }
            ;
function    : KW_LET name param_list '=' item {
                  $$ = Node_new(FUNCTION);
                  Node_append_child(&$$, $2);
                  Node_append_child(&$$, $3);
                  Node item_list = Node_new(ITEM_LIST);
                  Node_append_child(&item_list, $5);
                  Node_append_child(&$$, item_list);
              }
            | KW_LET name param_list '=' '{' item_list '}' {
                  $$ = Node_new(FUNCTION);
                  Node_append_child(&$$, $2);
                  Node_append_child(&$$, $3);
                  Node_append_child(&$$, $6);
              }
            ;
const_var   : KW_LET name '=' expr {
                  $$ = Node_new(CONST_VAR);
                  Node_append_child(&$$, $2);
                  Node_append_child(&$$, $4);
              }
            ;
name        : K_IDENT {
                  $$ = Node_new(NAME);
                  Node_set_text(&$$, $1);
              }
            ;
param_list  : '(' ')'       { $$ = Node_new(PARAM_LIST); }
            | param_list_     { $$ = $1; }
            ;
param_list_ :               { $$ = Node_new(PARAM_LIST); }
            | param_list_ param {
                  Node_append_child(&$1, $2);
                  $$ = $1;
              }
            ;
param       : K_IDENT {
                  $$ = Node_new(PARAM);
                  Node_set_text(&$$, $1);
              }
            ;
expr_list   :                { $$ = Node_new(EXPR_LIST); }
            | expr_list expr {
                  Node_append_child(&$1, $2);
                  $$ = $1;
              }
            | expr_list ',' expr {
                  Node_append_child(&$1, $3);
                  $$ = $1;
              }
            ;
expr        : if           { $$ = Node_new(EXPR); Node_append_child(&$$, $1); }
            | expr_op      { $$ = Node_new(EXPR); Node_append_child(&$$, $1); }
            | '(' expr ')' { $$ = $2; }
            | func_call    { $$ = Node_new(EXPR); Node_append_child(&$$, $1); }
            | number       { $$ = Node_new(EXPR); Node_append_child(&$$, $1); }
            | var_ref      { $$ = Node_new(EXPR); Node_append_child(&$$, $1); }
            ;
if          : KW_IF expr '{' item_list '}' {
                  $$ = Node_new(IF);
                  Node_append_child(&$$, $2);
                  Node_append_child(&$$, $4);
              }
            | KW_IF expr '{' item_list '}' KW_ELSE '{' item_list '}' {
                  $$ = Node_new(IF);
                  Node_append_child(&$$, $2);
                  Node_append_child(&$$, $4);
                  Node_append_child(&$$, $8);
              }
            ;
expr_op     : expr SYM_OR expr    { $$ = Node_new(OR);  Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_AND expr   { $$ = Node_new(AND); Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_EQ expr    { $$ = Node_new(EQ);  Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_NE expr    { $$ = Node_new(NE);  Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_LT expr    { $$ = Node_new(LT);  Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_GT expr    { $$ = Node_new(GT);  Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_LE expr    { $$ = Node_new(LE);  Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_GE expr    { $$ = Node_new(GE);  Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_PLUS expr  { $$ = Node_new(ADD); Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_MINUS expr { $$ = Node_new(SUB); Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_STAR expr  { $$ = Node_new(MUL); Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | expr SYM_SLASH expr { $$ = Node_new(DIV); Node_append_child(&$$, $1); Node_append_child(&$$, $3); }
            | SYM_BANG expr                { $$ = Node_new(NOT); Node_append_child(&$$, $2); }
            | SYM_MINUS expr %prec _UMINUS { $$ = Node_new(NEG); Node_append_child(&$$, $2); }
            ;
func_call   : expr '(' expr_list ')' {
                  $$ = Node_new(FUNC_CALL);
                  Node_append_child(&$$, $1);
                  Node_append_child(&$$, $3);
              }
            ;
var_ref     : K_IDENT {
                  $$ = Node_new(VAR_REF);
                  Node_set_text(&$$, $1);
              }
            ;
number      : K_INT {
                  $$ = Node_new(NUMBER);
                  Node_set_number(&$$, $1);
              }
            | K_FLOAT {
                  $$ = Node_new(NUMBER);
                  Node_set_number(&$$, $1);
              }
            ;
