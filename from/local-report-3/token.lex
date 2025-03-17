/* lex f3.lex */
alpha   [a-zA-Z]
digit   [0-9]
white   [\n\t ]
%%
let                             { return KW_LET; }
if                              { return KW_IF; }
else                            { return KW_ELSE; }
"=="                            { return SYM_EQ; }
"!="                            { return SYM_NE; }
"<="                            { return SYM_LE; }
">="                            { return SYM_GE; }
"||"                            { return SYM_OR; }
"&&"                            { return SYM_AND; }
"+"                             { return SYM_PLUS; }
"-"                             { return SYM_MINUS; }
"*"                             { return SYM_STAR; }
"/"                             { return SYM_SLASH; }
"!"                             { return SYM_BANG; }
"<"                             { return SYM_LT; }
">"                             { return SYM_GT; }
[=();{},]                       { return yytext[0]; }
(_|{alpha})(_|{alpha}|{digit})* { yylval.string = strdup(yytext); return K_IDENT; }
{digit}+"."{digit}+             { yylval.number = atof(yytext); return K_FLOAT; }
{digit}+                        { yylval.number = atof(yytext); return K_INT; }
({white}|"//"[^\n]*)            { ; }
