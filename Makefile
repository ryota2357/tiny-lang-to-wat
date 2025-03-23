CC     := clang
CFLAGS := -Wall -Wextra -Wshadow -ftrapv -fstack-protector-all -fsanitize=address,undefined -fno-sanitize-recover

CACHE := .cache
BIN   := bin

FLEX_PREFIX  := lex.yy
BISON_PREFIX := y.tab

.PHONY: all
all: $(BIN)/eval $(BIN)/compile

.PHONY: clean
clean:
	@rm -rf $(CACHE)
	@rm -f $(FLEX_PREFIX).*
	@rm -f $(BISON_PREFIX).*

$(BIN)/eval: $(BIN) $(CACHE)/main_eval.o $(CACHE)/$(FLEX_PREFIX).o $(CACHE)/$(BISON_PREFIX).o
	@$(CC) -ll $(CFLAGS) -o $@ $(wordlist 2, $(words $^), $^)

$(BIN)/compile: $(BIN) $(CACHE)/main_compile.o $(CACHE)/$(FLEX_PREFIX).o $(CACHE)/$(BISON_PREFIX).o
	@$(CC) -ll $(CFLAGS) -o $@ $(wordlist 2, $(words $^), $^)

$(CACHE)/%.o: %.c $(CACHE)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BISON_PREFIX).c $(BISON_PREFIX).h &: grammar.y
	@bison --output=$(BISON_PREFIX).c --header=$(BISON_PREFIX).h $<

$(FLEX_PREFIX).c $(FLEX_PREFIX).h &: token.l $(BISON_PREFIX).h
	@flex --outfile=$(FLEX_PREFIX).c --header-file=$(FLEX_PREFIX).h $<

$(CACHE) $(BIN):
	@mkdir $@
