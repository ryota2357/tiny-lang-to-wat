CC     := clang
CFLAGS := -ll -Wall -Wextra -Wshadow -ftrapv -fstack-protector-all -fsanitize=address,undefined -fno-sanitize-recover -g2

YACC_FILE    := grammar.y
LEX_FILE     := token.lex
WAT_FILE     := main.wat
MAIN_C_FILE  := main.c
IMPL_C_FILES := eval.c compile.c
INPUT_FILE   := input.txt

YACC_OUTPUT := y.tab.c
LEX_OUTPUT  := lex.yy.c
WASM_BINARY := main.wasm
EXECUTABLE  := main.out

.PHONY: all
all: $(EXECUTABLE)
	@MallocNanoZone=0 ./$(EXECUTABLE) < $(INPUT_FILE)

# Yacc (Bison) の生成
$(YACC_OUTPUT): $(YACC_FILE)
	@bison -o $(YACC_OUTPUT) $(YACC_FILE)

# Lex (Flex) の生成
$(LEX_OUTPUT): $(LEX_FILE)
	@flex -o $(LEX_OUTPUT) $(LEX_FILE)

# Wasm の生成
$(WASM_BINARY): $(WAT_FILE)
	@wat2wasm $(WAT_FILE) -o $(WASM_BINARY)

# コンパイル
$(EXECUTABLE): $(YACC_OUTPUT) $(LEX_OUTPUT) $(MAIN_C_FILE) $(IMPL_C_FILES)
	@$(CC) $(CFLAGS) $(MAIN_C_FILE) -o $(EXECUTABLE)

.PHONY: clean
clean:
	@rm -f $(YACC_OUTPUT) $(YACC_HEADER) $(LEX_OUTPUT) $(EXECUTABLE)
