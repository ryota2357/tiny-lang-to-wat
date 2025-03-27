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

.PHONY: examples
examples: $(BIN)/compile
	@for dir in examples/*; do \
	  if [ -d "$$dir" ]; then \
	    name=$$(basename $$dir); \
	    if [ ! -f "$$dir/main.tl2w" ]; then \
	      echo "Error: $$dir/main.tl2w not found"; \
	      continue; \
	    fi; \
	    echo "Building example: $$name"; \
	    MallocNanoZone=0 ./$(BIN)/compile "$$dir/main.tl2w" > "$$dir/main.wat"; \
	    wat2wasm "$$dir/main.wat" -o "$$dir/main.wasm"; \
	  fi \
	done

$(BIN)/eval: $(CACHE)/main_eval.o $(CACHE)/$(FLEX_PREFIX).o $(CACHE)/$(BISON_PREFIX).o | $(BIN)
	$(CC) -ll $(CFLAGS) -o $@ $^

$(BIN)/compile: $(CACHE)/main_compile.o $(CACHE)/$(FLEX_PREFIX).o $(CACHE)/$(BISON_PREFIX).o | $(BIN)
	@$(CC) -ll $(CFLAGS) -o $@ $^

$(CACHE)/main_%.o: main_%.c $(BISON_PREFIX).h $(FLEX_PREFIX).h | $(CACHE)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(CACHE)/%.o: %.c | $(CACHE)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BISON_PREFIX).c $(BISON_PREFIX).h &: grammar.y
	@bison --output=$(BISON_PREFIX).c --header=$(BISON_PREFIX).h $<

$(FLEX_PREFIX).c $(FLEX_PREFIX).h &: token.l $(BISON_PREFIX).h
	@flex --outfile=$(FLEX_PREFIX).c --header-file=$(FLEX_PREFIX).h $<

$(CACHE) $(BIN):
	@mkdir -p $@
