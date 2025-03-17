/* #include "eval.c" */
#include "compile.c"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#include "y.tab.c"
#include "lex.yy.c"
#pragma clang diagnostic pop

int main() {
    yyparse();
    return 0;
}
