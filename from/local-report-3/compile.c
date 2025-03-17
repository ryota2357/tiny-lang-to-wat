#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *yytext;

typedef enum {
    // .children
    ITEM_LIST,
    ITEM,
    FUNCTION,
    CONST_VAR,
    FUNC_CALL,
    ARG_LIST,
    EXPR_LIST,
    EXPR,
    IF,
    OR,
    AND,
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
    ADD,
    SUB,
    MUL,
    DIV,
    NOT,
    NEG,

    ARG,      // .text
    NAME,     // .text
    VAR_REF,  // .text
    NUMBER,   // .value
} SyntaxKind;

#define DEF_DYNAMICS_ARRAY(name, elem_type) \
    typedef struct {                        \
        elem_type* _buf;                    \
        size_t len;                         \
        size_t _cap;                        \
    } name;                                 \
    name name##_new() {                     \
        return (name){                      \
            ._buf = NULL,                   \
            .len = 0,                       \
            ._cap = 0,                      \
        };                                  \
    }

typedef double Number;

DEF_DYNAMICS_ARRAY(NodeArray, struct Node);

typedef union NodeData {
    NodeArray children;
    Number number;
    char* text;
} NodeData;

typedef struct Node {
    SyntaxKind kind;
    NodeData data;
} Node;

void NodeArray_push_back(NodeArray* self, Node node) {
    if (self->len == self->_cap) {
        self->_cap = self->_cap == 0 ? 1 : self->_cap * 2;
        self->_buf = (Node*)realloc(self->_buf, sizeof(Node) * self->_cap);
    }
    self->_buf[self->len++] = node;
};
Node NodeArray_get(NodeArray* self, size_t index) {
    assert(index < self->len);
    return self->_buf[index];
}

Node Node_new(SyntaxKind kind) {
    NodeData data;
    switch (kind) {
        case NAME:
        case ARG:
            data.text = "";
            break;
        case NUMBER:
            data.number = 0;
            break;
        default:
            data.children = NodeArray_new();
            break;
    }
    return (Node){
        .kind = kind,
        .data = data,
    };
}
void Node_set_text(Node* self, char* text) {
    assert(self->kind == NAME || self->kind == ARG || self->kind == VAR_REF);
    self->data.text = text;
}
void Node_set_number(Node* self, Number number) {
    assert(self->kind == NUMBER);
    self->data.number = number;
}
void Node_append_child(Node* self, Node child) {
    assert(self->kind != NAME && self->kind != NUMBER && self->kind != ARG);
    NodeArray_push_back(&self->data.children, child);
}

typedef struct {
    char* text;
    size_t len;
    uint64_t offset;
} StringMapEntry;
DEF_DYNAMICS_ARRAY(StringMap, StringMapEntry);
uint64_t StringMap_get_or_insert(StringMap* self, char* text) {
    size_t len = strlen(text);
    for (size_t i = 0; i < self->len; ++i) {
        StringMapEntry entry = self->_buf[i];
        if (entry.len == len && strcmp(entry.text, text) == 0) {
            return entry.offset << 32 | len;
        }
    }
    uint64_t offset = self->len ? (self->_buf[self->len - 1].offset + self->_buf[self->len - 1].len) : 0;
    uint64_t text_id = offset << 32 | len; // text_id: | ptr (4) | len (4) |
    if (self->len == self->_cap) {
        self->_cap = self->_cap == 0 ? 1 : self->_cap * 2;
        self->_buf = (StringMapEntry*)realloc(self->_buf, sizeof(StringMapEntry) * self->_cap);
    }
    self->_buf[self->len].text = (char*)malloc(sizeof(char) * (len + 1));
    memcpy(self->_buf[self->len].text, text, len);
    self->_buf[self->len].text[len] = '\0';
    self->_buf[self->len].len = len;
    self->_buf[self->len].offset = offset;
    self->len += 1;
    return text_id;
}
char* StringMap_get_linearized_escaped_text(StringMap* self) {
    int excape_count = 0;
    for (size_t i = 0; i < self->len; ++i) {
        StringMapEntry entry = self->_buf[i];
        for (size_t j = 0; j < entry.len; ++j) {
            if (entry.text[j] == '\n' || entry.text[j] == '\t' || entry.text[j] == '"') {
                excape_count++;
            }
        }
    }
    size_t non_escaped_len = self->_buf[self->len - 1].offset + self->_buf[self->len - 1].len;
    size_t text_len = non_escaped_len + excape_count;
    char* text = (char*)malloc(sizeof(char) * (text_len + 1));
    size_t text_i = 0;
    for (size_t i = 0; i < self->len; ++i) {
        StringMapEntry entry = self->_buf[i];
        for (size_t j = 0; j < entry.len; ++j) {
            char c = entry.text[j];
            switch (c) {
                case '\n':
                    text[text_i++] = '\\';
                    text[text_i++] = 'n';
                    break;
                case '\t':
                    text[text_i++] = '\\';
                    text[text_i++] = 't';
                    break;
                case '"':
                    text[text_i++] = '\\';
                    text[text_i++] = '"';
                    break;
                default:
                    text[text_i++] = c;
                    break;
            }
        }
    }
    text[text_len] = '\0';
    return text;
}

DEF_DYNAMICS_ARRAY(StringBuilder, char);
void StringBuilder_append(StringBuilder* self, char* text) {
    size_t len = strlen(text);
    if (self->len + len + 1 > self->_cap) {
        size_t new_capacity = self->_cap == 0 ? 1 : self->_cap * 2;
        while (self->len + len + 1 > new_capacity) {
            new_capacity *= 2;
        }
        self->_cap = new_capacity;
        self->_buf = (char*)realloc(self->_buf, sizeof(char) * self->_cap);
    }
    memcpy(self->_buf + self->len, text, len);
    self->len += len;
    self->_buf[self->len] = '\0';
}
void StringBuilder_appendf(StringBuilder* self, size_t buf_size, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char* buf = (char*)calloc(buf_size, sizeof(char));
    vsnprintf(buf, buf_size * sizeof(char), fmt, args);
    va_end(args);
    StringBuilder_append(self, buf);
}
char* StringBuilder_to_string(StringBuilder* self) {
    char* text = (char*)malloc(self->len + 1);
    memcpy(text, self->_buf, self->len);
    text[self->len] = '\0';
    return text;
}
void StringBuilder_clear(StringBuilder* self) {
    free(self->_buf);
    self->_buf = NULL;
    self->len = 0;
    self->_cap = 0;
}

typedef struct {
    int id;
    int arg_count;
    StringBuilder code_sb;
} Function;
Function Function_new(int id, int arg_count) {
    StringBuilder sb = StringBuilder_new();
    StringBuilder_appendf(&sb, 64, "(func $f%d (result i32)\n", id);
    return (Function){
        .id = id,
        .arg_count = arg_count,
        .code_sb = sb,
    };
}
char* Function_to_string(Function* self) {
    char* code = StringBuilder_to_string(&self->code_sb);
    size_t len = strlen(code);
    code = (char*)realloc(code, sizeof(char) * (len + 3));
    code[len] = ')';
    code[len + 1] = '\n';
    code[len + 2] = '\0';
    return code;
}

DEF_DYNAMICS_ARRAY(FunctionArray, Function);
Function FunctionArray_get(FunctionArray* self, size_t index) {
    assert(index < self->len);
    return self->_buf[index];
}
void FunctionArray_push_back(FunctionArray* self, Function func) {
    if (self->len == self->_cap) {
        self->_cap = self->_cap == 0 ? 1 : self->_cap * 2;
        self->_buf = (Function*)realloc(self->_buf, sizeof(Function) * self->_cap);
    }
    self->_buf[self->len++] = func;
}
int __function_cmp(const void* a, const void* b) {
    return ((Function*)a)->id - ((Function*)b)->id;
}
void FunctionArray_sort_by_id(FunctionArray* self) {
    qsort(self->_buf, self->len, sizeof(Function), __function_cmp);
}

typedef struct {
    StringMap ident_map;
    int next_func_id;
    FunctionArray functions;
} Context;
Context Context_new() {
    return (Context){
        .ident_map = StringMap_new(),
        .next_func_id = 0,
        .functions = FunctionArray_new(),
    };
}

char* compile_item(Context *ctx, Node node, bool is_tail);

StringBuilder compile_expr(Context *ctx, Node node) {
    assert(node.kind == EXPR);
    Node expr = NodeArray_get(&node.data.children, 0);
    char* bin_op_code;
    switch (expr.kind) {
        case FUNC_CALL: {
            StringBuilder sb = StringBuilder_new();

            // push scope frame
            StringBuilder_append(&sb, "global.get $env_top\n");

            // push arguments as $a0, $a1, ...
            Node expr_list = NodeArray_get(&expr.data.children, 1);
            assert(expr_list.kind == EXPR_LIST);
            for (size_t i = 0; i < expr_list.data.children.len; ++i) {
                char a_name[32] = {0};
                snprintf(a_name, sizeof(a_name), "$a%ld", i);
                uint64_t a_id = StringMap_get_or_insert(&ctx->ident_map, a_name);
                StringBuilder_appendf(&sb, 64, "i64.const %lld  ;; %s\n", (int64_t)a_id, a_name);

                Node arg = NodeArray_get(&expr_list.data.children, i);
                StringBuilder asb = compile_expr(ctx, arg);
                StringBuilder_append(&sb, asb._buf);
                StringBuilder_clear(&asb);

                StringBuilder_append(&sb, "call $set_value\n");
            }

            // call function
            Node func = NodeArray_get(&expr.data.children, 0);
            StringBuilder fsb = compile_expr(ctx, func);
            StringBuilder_append(&sb, fsb._buf);
            StringBuilder_clear(&fsb);
            StringBuilder_append(&sb, "call $typecheck_fun\n");
            StringBuilder_append(&sb, "call_indirect (type $return_i32)\n");

            // pop scope frame
            // To keep the return value of function call, use $i32_stash.
            StringBuilder_append(&sb, "global.set $i32_stash\n");
            StringBuilder_append(&sb, "global.set $env_top\n");
            StringBuilder_append(&sb, "global.get $i32_stash\n");
            return sb;
        }
        case IF: {
            // compile condition
            Node cond = NodeArray_get(&expr.data.children, 0);
            StringBuilder sb = compile_expr(ctx, cond);
            StringBuilder_append(&sb, "call $typecheck_f64\n");

            // We want to interpret the if condition as false if all 8 bytes (pushed above) are zero, otherwise true.
            // However, WASM only accepts i32 for if condition so we need to convert it to i32.
            StringBuilder_append(&sb, "f64.abs\n"); // remove sign bit
            StringBuilder_append(&sb, "i64.reinterpret_f64\n");
            StringBuilder_append(&sb, "i64.eqz\n");
            StringBuilder_append(&sb, "i32.eqz\n");

            // Each branch block in a WASM if-else statement must not leave a value on the stack.
            // Now we have to compile if "expresion", need to return the value (need to leave value on the stack).
            // So use $i32_stash (a global variable) to store the return value and reload it after the if block.
            StringBuilder_append(&sb, "if\n");
            Node then = NodeArray_get(&expr.data.children, 1);
            assert(then.kind == ITEM_LIST);
            for (size_t i = 0; i < then.data.children.len; ++i) {
                // compile then block
                Node item = NodeArray_get(&then.data.children, i);
                char* code = compile_item(ctx, item, i == then.data.children.len - 1);
                StringBuilder_append(&sb, code);
                StringBuilder_append(&sb, "global.set $i32_stash\n");
                free(code);
            }
            StringBuilder_append(&sb, "else\n");
            if (expr.data.children.len == 3) {
                // compile else block
                Node els = NodeArray_get(&expr.data.children, 2);
                assert(els.kind == ITEM_LIST);
                for (size_t i = 0; i < els.data.children.len; ++i) {
                    Node item = NodeArray_get(&els.data.children, i);
                    char* code = compile_item(ctx, item, i == els.data.children.len - 1);
                    StringBuilder_append(&sb, code);
                    free(code);
                }
                StringBuilder_append(&sb, "global.set $i32_stash\n");
            } else {
                // if no else block, return 0
                StringBuilder_append(&sb, "f64.const 0\n");
                StringBuilder_append(&sb, "global.set $value_f64\n");
                StringBuilder_append(&sb, "i32.const 0\n");
                StringBuilder_append(&sb, "global.set $i32_stash\n");
            }
            StringBuilder_append(&sb, "end\n");
            StringBuilder_append(&sb, "global.get $i32_stash\n");
            return sb;
        }
        case OR: {
            // compile lhs
            Node lhs = NodeArray_get(&expr.data.children, 0);
            StringBuilder lsb = compile_expr(ctx, lhs);
            StringBuilder_append(&lsb, "call $typecheck_f64\n");

            // The meaning of this code is explained within `case IF:` above.
            StringBuilder_append(&lsb, "f64.abs\n");
            StringBuilder_append(&lsb, "i64.reinterpret_f64\n");
            StringBuilder_append(&lsb, "i64.eqz\n");
            StringBuilder_append(&lsb, "i32.eqz\n");

            // short-circuit evaluation
            // The reason why we use $i32_stash is explained within `case IF:` above.
            StringBuilder_append(&lsb, "if\n");
            StringBuilder_append(&lsb, "f64.const 1\n");
            StringBuilder_append(&lsb, "global.set $value_f64\n");
            StringBuilder_append(&lsb, "i32.const 0\n");
            StringBuilder_append(&lsb, "global.set $i32_stash\n");
            StringBuilder_append(&lsb, "else\n");
            {
                Node rhs = NodeArray_get(&expr.data.children, 1);
                StringBuilder rsb = compile_expr(ctx, rhs);
                StringBuilder_append(&lsb, rsb._buf);
                StringBuilder_clear(&rsb);
                StringBuilder_append(&lsb, "global.set $i32_stash\n");
            }
            StringBuilder_append(&lsb, "end\n");
            StringBuilder_append(&lsb, "global.get $i32_stash\n");
            return lsb;
        }
        case AND: {
            // compile lhs
            Node lhs = NodeArray_get(&expr.data.children, 0);
            StringBuilder lsb = compile_expr(ctx, lhs);
            StringBuilder_append(&lsb, "call $typecheck_f64\n");

            // The meaning of this code is explained within `case IF:` above.
            StringBuilder_append(&lsb, "f64.abs\n");
            StringBuilder_append(&lsb, "i64.reinterpret_f64\n");
            StringBuilder_append(&lsb, "i64.eqz\n");
            StringBuilder_append(&lsb, "i32.eqz\n");

            // short-circuit evaluation
            // The reason why we use $i32_stash is explained within `case IF:` above.
            StringBuilder_append(&lsb, "if\n");
            {
                Node rhs = NodeArray_get(&expr.data.children, 1);
                StringBuilder rsb = compile_expr(ctx, rhs);
                StringBuilder_append(&lsb, rsb._buf);
                StringBuilder_clear(&rsb);
                StringBuilder_append(&lsb, "global.set $i32_stash\n");
            }
            StringBuilder_append(&lsb, "else\n");
            StringBuilder_append(&lsb, "f64.const 0\n");
            StringBuilder_append(&lsb, "global.set $value_f64\n");
            StringBuilder_append(&lsb, "i32.const 0\n");
            StringBuilder_append(&lsb, "global.set $i32_stash\n");
            StringBuilder_append(&lsb, "end\n");
            StringBuilder_append(&lsb, "global.get $i32_stash\n");
            return lsb;
        }
        case EQ: bin_op_code = "f64.eq\nf64.convert_i32_u\n"; goto BIN_OP;
        case NE: bin_op_code = "f64.ne\nf64.convert_i32_u\n"; goto BIN_OP;
        case LT: bin_op_code = "f64.lt\nf64.convert_i32_u\n"; goto BIN_OP;
        case GT: bin_op_code = "f64.gt\nf64.convert_i32_u\n"; goto BIN_OP;
        case LE: bin_op_code = "f64.le\nf64.convert_i32_u\n"; goto BIN_OP;
        case GE: bin_op_code = "f64.ge\nf64.convert_i32_u\n"; goto BIN_OP;
        case ADD: bin_op_code = "f64.add\n"; goto BIN_OP;
        case SUB: bin_op_code = "f64.sub\n"; goto BIN_OP;
        case MUL: bin_op_code = "f64.mul\n"; goto BIN_OP;
        case DIV: {
            bin_op_code = "f64.div\n";
            BIN_OP: {
                Node lhs = NodeArray_get(&expr.data.children, 0);
                StringBuilder lsb = compile_expr(ctx, lhs);
                StringBuilder_append(&lsb, "call $typecheck_f64\n");
                {
                    Node rhs = NodeArray_get(&expr.data.children, 1);
                    StringBuilder rsb = compile_expr(ctx, rhs);
                    StringBuilder_append(&lsb, rsb._buf);
                    StringBuilder_clear(&rsb);
                }
                StringBuilder_append(&lsb, "call $typecheck_f64\n");
                StringBuilder_append(&lsb, bin_op_code);
                StringBuilder_append(&lsb, "global.set $value_f64\n");
                StringBuilder_append(&lsb, "i32.const 0\n");
                return lsb;
            }
        }
        case NOT: {
            StringBuilder sb = compile_expr(ctx, NodeArray_get(&expr.data.children, 0));
            StringBuilder_append(&sb, "call $typecheck_f64\n");
            StringBuilder_append(&sb, "f64.abs\n");
            StringBuilder_append(&sb, "i64.reinterpret_f64\n");
            StringBuilder_append(&sb, "i64.eqz\n");
            StringBuilder_append(&sb, "f64.convert_i32_u\n");
            StringBuilder_append(&sb, "global.set $value_f64\n");
            StringBuilder_append(&sb, "i32.const 0\n");
            return sb;
        }
        case NEG: {
            StringBuilder sb = compile_expr(ctx, NodeArray_get(&expr.data.children, 0));
            StringBuilder_append(&sb, "call $typecheck_f64\n");
            StringBuilder_append(&sb, "f64.const -1\n");
            StringBuilder_append(&sb, "f64.mul\n");
            StringBuilder_append(&sb, "global.set $value_f64\n");
            StringBuilder_append(&sb, "i32.const 0\n");
            return sb;
        }
        case VAR_REF: {
            StringBuilder sb = StringBuilder_new();
            uint64_t id = StringMap_get_or_insert(&ctx->ident_map, expr.data.text);
            StringBuilder_appendf(&sb, 64, "i64.const %lld  ;; %s\n", (int64_t)id, expr.data.text);
            StringBuilder_append(&sb, "call $get_value\n");
            return sb;
        }
        case NUMBER: {
            StringBuilder sb = StringBuilder_new();
            StringBuilder_appendf(&sb, 64, "f64.const %f\n", expr.data.number);
            StringBuilder_append(&sb, "global.set $value_f64\n");
            StringBuilder_append(&sb, "i32.const 0\n");
            return sb;
        }
        case EXPR: fprintf(stderr, "bug: nested EXPR\n");
        case ARG: fprintf(stderr, "bug: ARG in EXPR\n");
        case NAME: fprintf(stderr, "bug: NAME in EXPR\n");
        case ARG_LIST: fprintf(stderr, "bug: ARG_LIST in EXPR\n");
        case EXPR_LIST: fprintf(stderr, "bug: EXPR_LIST in EXPR\n");
        case ITEM_LIST: fprintf(stderr, "bug: ITEM_LIST in EXPR\n");
        case ITEM: fprintf(stderr, "bug: ITEM in EXPR\n");
        case FUNCTION: fprintf(stderr, "bug: FUNCTION in EXPR\n");
        case CONST_VAR: fprintf(stderr, "bug: CONST_VAR in EXPR\n");
        default:
            assert(false);
    }
    assert(false);
}

int compile_function(Context *ctx, Node node) {
    assert(node.kind == FUNCTION);

    Node arg_list = NodeArray_get(&node.data.children, 1);
    assert(arg_list.kind == ARG_LIST);
    Function func = Function_new(ctx->next_func_id++, arg_list.data.children.len);

    // Arguments are stored $a0, $a1, ... .
    // We need to re-assign them to the variable (actual arguments's) name.
    for (size_t i = 0; i < arg_list.data.children.len; ++i) {
        Node arg = NodeArray_get(&arg_list.data.children, i);
        assert(arg.kind == ARG);
        uint64_t id = StringMap_get_or_insert(&ctx->ident_map, arg.data.text);
        StringBuilder_appendf(&func.code_sb, 64, "i64.const %lld  ;; %s\n", (int64_t)id, arg.data.text);

        char a_name[32] = {0};
        snprintf(a_name, sizeof(a_name), "$a%ld", i);
        uint64_t a_id = StringMap_get_or_insert(&ctx->ident_map, a_name);
        StringBuilder_appendf(&func.code_sb, 64, "i64.const %lld  ;; %s\n", (int64_t)a_id, a_name);
        StringBuilder_append(&func.code_sb, "call $get_value\n");

        StringBuilder_append(&func.code_sb, "call $set_value\n");
    }

    // compile function body
    Node item_list = NodeArray_get(&node.data.children, 2);
    assert(item_list.kind == ITEM_LIST);
    for (size_t i = 0; i < item_list.data.children.len; ++i) {
        Node item = NodeArray_get(&item_list.data.children, i);
        char* code = compile_item(ctx, item, i == item_list.data.children.len - 1);
        StringBuilder_append(&func.code_sb, code);
        free(code);
    }

    FunctionArray_push_back(&ctx->functions, func);
    return func.id;
}

char* compile_item(Context *ctx, Node node, bool is_tail) {
    assert(node.kind == ITEM);
    Node item = NodeArray_get(&node.data.children, 0);
    switch (item.kind) {
        case FUNCTION: {
            StringBuilder sb = StringBuilder_new();

            // load function name onto the stack
            Node name_node = NodeArray_get(&item.data.children, 0);
            assert(name_node.kind == NAME);
            char* name = name_node.data.text;
            uint64_t id = StringMap_get_or_insert(&ctx->ident_map, name);
            StringBuilder_appendf(&sb, 64, "i64.const %lld  ;; %s\n", (int64_t)id, name);

            // compile function (anonymous) and load its reference onto the stack
            int func_id = compile_function(ctx, item);
            StringBuilder_appendf(&sb, 64, "i32.const %d\n", func_id);
            StringBuilder_append(&sb, "global.set $value_fun\n");
            StringBuilder_append(&sb, "i32.const 1\n");

            // name function reference
            StringBuilder_append(&sb, "call $set_value\n");
            if (is_tail) {
                // If tail of item_list (such as function body), load the value onto the stack as return value.
                StringBuilder_appendf(&sb, 64, "i64.const %lld  ;; %s\n", (int64_t)id, name);
                StringBuilder_append(&sb,"call $get_value\n");
            }

            char* res = StringBuilder_to_string(&sb);
            StringBuilder_clear(&sb);
            return res;
        }
        case CONST_VAR: {
            StringBuilder sb = StringBuilder_new();

            // load variable name onto the stack
            Node name = NodeArray_get(&item.data.children, 0);
            assert(name.kind == NAME);
            uint64_t id = StringMap_get_or_insert(&ctx->ident_map, name.data.text);
            StringBuilder_appendf(&sb, 64, "i64.const %lld  ;; %s\n", (int64_t)id, name.data.text);

            // append expression code
            Node expr = NodeArray_get(&item.data.children, 1);
            StringBuilder esb = compile_expr(ctx, expr);
            StringBuilder_append(&sb, esb._buf);
            StringBuilder_clear(&esb);

            // store value as a named variable
            StringBuilder_append(&sb, "call $set_value\n");
            if (is_tail) {
                // If tail of item_list (such as function body), load the value onto the stack as return value.
                StringBuilder_appendf(&sb, 64, "i64.const %lld ;; %s\n", (int64_t)id, name.data.text);
                StringBuilder_append(&sb, "call $get_value\n");
            }

            char* res = StringBuilder_to_string(&sb);
            StringBuilder_clear(&sb);
            return res;
        }
        case EXPR: {
            StringBuilder sb = compile_expr(ctx, item);
            if (!is_tail) {
                // If not tail of item_list, it means the expression is a statement so we need to un-load the value from the stack.
                StringBuilder_append(&sb, "drop\n");
            }
            char* res = StringBuilder_to_string(&sb);
            StringBuilder_clear(&sb);
            return res;
        }
        default: assert(false);
    }
}

void start(Node root) {
    assert(root.kind == ITEM_LIST);

    Context ctx = Context_new();
    StringBuilder main_code_sb = StringBuilder_new();
    {
        // insert `print` function
        uint64_t id = StringMap_get_or_insert(&ctx.ident_map, "print");
        char load_id[64] = {0};
        snprintf(load_id, sizeof(load_id), "i64.const %lld  ;; print\n", (int64_t)id);
        StringBuilder_append(&main_code_sb, load_id);

        Function func = Function_new(ctx.next_func_id++, 1);
        uint64_t a_id = StringMap_get_or_insert(&ctx.ident_map, "$a0");
        StringBuilder_appendf(&func.code_sb, 32, "i64.const %lld  ;; $a0\n", (int64_t)a_id);
        StringBuilder_append(&func.code_sb, "call $get_value\n");
        StringBuilder_append(&func.code_sb, "call $typecheck_f64\n"); // TODO: impl print function
        StringBuilder_append(&func.code_sb, "call $write_f64\n");
        StringBuilder_append(&func.code_sb, "i32.const 0\n");
        FunctionArray_push_back(&ctx.functions, func);

        StringBuilder_appendf(&main_code_sb, 32, "i32.const %d\n", func.id);
        StringBuilder_append(&main_code_sb, "global.set $value_fun\n");
        StringBuilder_append(&main_code_sb, "i32.const 1\n");
        StringBuilder_append(&main_code_sb, "call $set_value\n");
    }
    for (size_t i = 0; i < root.data.children.len; ++i) {
        Node item = NodeArray_get(&root.data.children, i);
        char* code = compile_item(&ctx, item, false);
        StringBuilder_append(&main_code_sb, code);
    }

    uint64_t msg_unexpected_f64_id = StringMap_get_or_insert(&ctx.ident_map, "Unexpected number\n");
    uint64_t msg_unexpected_fun_id = StringMap_get_or_insert(&ctx.ident_map, "Unexpected function\n");
    uint64_t msg_undefined_varialbe_prefix_id = StringMap_get_or_insert(&ctx.ident_map, "error: `");
    uint64_t msg_undefined_varialbe_suffix_id = StringMap_get_or_insert(&ctx.ident_map, "` is not defined\n");

    printf("(module\n");
    printf( // import functions
        "  (func $write_str (import \"imports\" \"write_str\") (param i32) (param i32))\n"
        "  (func $write_f64 (import \"imports\" \"write_f64\") (param f64))\n"
        "  (func $exit      (import \"imports\" \"exit\") (param i32))\n"
    );
    {
        // memory
        char* text = StringMap_get_linearized_escaped_text(&ctx.ident_map);
        printf(
            "  (memory 16)\n"
            "  (export \"memory\" (memory 0))\n"
            "  (data (i32.const 0) \"%s\")\n",
            text
        );

        // globals
        uint64_t text_memory_size = strlen(text);
        uint64_t env_base = (text_memory_size / 24) * 24 + (24 * 2);
        printf( // globals
            "  (global $i32_stash (mut i32) (i32.const 0))\n"
            "  (global $env_base i32 (i32.const %d))\n"
            "  (global $env_top (mut i32) (i32.const %d))\n"
            "  (global $value_f64 (mut f64) (f64.const 0))\n"
            "  (global $value_fun (mut i32) (i32.const 0))\n",
            (int32_t)env_base, (int32_t)env_base
        );
    }
    {
        // functions
        printf("  (type $return_i32 (func (result i32)))\n");
        printf("  (table %ld funcref)\n", ctx.functions.len);
        printf("  (elem (i32.const 0)");
        FunctionArray_sort_by_id(&ctx.functions);
        for (size_t i = 0; i < ctx.functions.len; ++i) {
            Function func = FunctionArray_get(&ctx.functions, i);
            printf(" $f%d", func.id);
        }
        printf(")\n");
        for (size_t i = 0; i < ctx.functions.len; ++i) {
            Function func = FunctionArray_get(&ctx.functions, i);
            printf("%s", Function_to_string(&func));
        }
    }
    {
        // hand written functions
        printf(
            "  (func $get_value (param $text_id i64) (result i32)\n"
            "    (local $ptr i32)\n"
            "    (local.set $ptr (global.get $env_top))\n"
            "    (block $break\n"
            "      (loop $continue\n"
            "        (local.set\n"
            "          $ptr\n"
            "          (i32.sub\n"
            "            (local.get $ptr)\n"
            "            (i32.const 24)))\n"
            "        (i32.lt_u\n"
            "          (local.get $ptr)\n"
            "          (global.get $env_base))\n"
            "        (br_if $break)\n"
            "        (i64.ne\n"
            "          (i64.load (local.get $ptr))\n"
            "          (local.get $text_id))\n"
            "        (br_if $continue)\n"
            "        (if\n"
            "          (i32.eqz (; flag == 0 ? f64 : fun ;)\n"
            "            (i32.load\n"
            "              (i32.add\n"
            "                (i32.const 8)\n"
            "                (local.get $ptr))))\n"
            "          (then\n"
            "            (global.set\n"
            "              $value_f64\n"
            "              (f64.load\n"
            "                (i32.add\n"
            "                  (local.get $ptr)\n"
            "                  (i32.const 16))))\n"
            "            (return (i32.const 0)))\n"
            "          (else\n"
            "            (global.set\n"
            "              $value_fun\n"
            "              (i32.load\n"
            "                (i32.add\n"
            "                  (local.get $ptr)\n"
            "                  (i32.const 20))))\n"
            "            (return (i32.const 1))))))\n"
            "    (call $write_str (i32.const %d) (i32.const %d)) ;; \"error: `\"\n"
            "    (call $write_str\n"
            "          (i32.wrap_i64 (i64.shr_u (local.get $text_id) (i64.const 32)))\n"
            "          (i32.wrap_i64 (local.get $text_id)))\n"
            "    (call $write_str (i32.const %d) (i32.const %d)) ;; \"` is not defined\"\n"
            "    (call $exit (i32.const 1))\n"
            "    (unreachable))\n",
            (int32_t)(msg_undefined_varialbe_prefix_id >> 32), (int32_t)(msg_undefined_varialbe_prefix_id & (uint32_t)-1),
            (int32_t)(msg_undefined_varialbe_suffix_id >> 32), (int32_t)(msg_undefined_varialbe_suffix_id & (uint32_t)-1)
        );
        printf(
            "  (func $set_value (param $text_id i64) (param $flag i32)\n"
            "    (i64.store (; store text_id ;)\n"
            "      (global.get $env_top)\n"
            "      (local.get $text_id))\n"
            "    (if\n"
            "      (local.get $flag)\n"
            "      (then (; fun ;)\n"
            "        (i32.store (; store flag ;)\n"
            "          (i32.add (global.get $env_top) (i32.const 8))\n"
            "          (i32.const 1))\n"
            "        (i32.store (; store funcref;)\n"
            "          (i32.add (global.get $env_top) (i32.const 20))\n"
            "          (global.get $value_fun)))\n"
            "      (else (; f64 ;)\n"
            "        (i32.store (; store flag ;)\n"
            "          (i32.add (global.get $env_top) (i32.const 8))\n"
            "          (i32.const 0))\n"
            "        (f64.store (; store f64;)\n"
            "          (i32.add (global.get $env_top) (i32.const 16))\n"
            "          (global.get $value_f64))))\n"
            "    (global.set $env_top (; update env_top ;)\n"
            "      (i32.add\n"
            "        (global.get $env_top)\n"
            "        (i32.const 24))))\n"
        );
        printf(
            "  (func $typecheck_f64 (param $flag i32) (result f64)\n"
            "    (if\n"
            "      (local.get $flag)\n"
            "      (then\n"
            "        (call $write_str (i32.const %d) (i32.const %d)) ;; \"Unexpected function\"\n"
            "        (call $exit (i32.const 1))\n"
            "        (unreachable)))\n"
            "    (return (global.get $value_f64)))\n",
            (int32_t)(msg_unexpected_fun_id >> 32), (int32_t)(msg_unexpected_fun_id & (uint32_t)-1)
        );
        printf(
            "  (func $typecheck_fun (param $flag i32) (result i32)\n"
            "    (if\n"
            "      (i32.eqz (local.get $flag))\n"
            "      (then\n"
            "        (call $write_str (i32.const %d) (i32.const %d)) ;; \"Unexpected number\"\n"
            "        (call $exit (i32.const 1))\n"
            "        (unreachable)))\n"
            "    (return (global.get $value_fun)))\n",
            (int32_t)(msg_unexpected_f64_id >> 32), (int32_t)(msg_unexpected_f64_id & (uint32_t)-1)
        );
    }
    {
        // main function
        char* main_code = StringBuilder_to_string(&main_code_sb);
        printf("  (func (export \"main\")\n%s)", main_code);
    }
    printf(")\n");
}
