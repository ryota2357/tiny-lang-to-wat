#include <assert.h>
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

typedef double Number;

typedef struct {
    struct Node* data;
    size_t size;
    size_t capacity;
} NodeArray;

typedef union NodeData {
    NodeArray children;
    Number number;
    char* text;
} NodeData;

typedef struct Node {
    SyntaxKind kind;
    NodeData data;
} Node;

NodeArray NodeArray_new() {
    return (NodeArray){
        .data = NULL,
        .size = 0,
        .capacity = 0,
    };
}
void NodeArray_push_back(NodeArray* self, Node node) {
    if (self->size == self->capacity) {
        self->capacity = self->capacity == 0 ? 1 : self->capacity * 2;
        self->data = (Node*)realloc(self->data, sizeof(Node) * self->capacity);
    }
    self->data[self->size++] = node;
};

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
    char** args;
    size_t args_size;
    NodeArray body;
} Function;

typedef enum {
    BUILTIN_FUNC_PRINT,
    /* BUILTIN_FUNC_READ, */
} BuiltinFunc;

typedef struct {
    enum {
        VALUE_KIND_NUMBER,        // .number
        VALUE_KIND_FUNC,          // .function
        VALUE_KIND_BUILTIN_FUNC,  // .builtin_func
    } kind;
    union {
        Number number;
        Function function;
        BuiltinFunc builtin_func;
    };
} Value;

typedef struct {
    char** names;
    Value* values;
    size_t size;
    size_t capacity;
} Env;

Env Env_new(size_t capacity) {
    return (Env){
        .names = (char**)malloc(sizeof(char*) * capacity),
        .values = (Value*)malloc(sizeof(Value) * capacity),
        .size = 0,
        .capacity = capacity,
    };
}
ssize_t Env_find(Env* const self, const char* const name) {
    if (self->size == 0) return -1;
    for (ssize_t i = self->size - 1; i >= 0; --i) {
        if (strcmp(self->names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}
Value Env_get_value(Env* const self, const char* const name) {
    ssize_t index = Env_find(self, name);
    if (index == -1) {
        if (strcmp(name, "print") == 0) {
            return (Value){
                .kind = VALUE_KIND_BUILTIN_FUNC,
                .builtin_func = BUILTIN_FUNC_PRINT,
            };
        } else {
            fprintf(stderr, "error: `%s` is not defined\n", name);
            exit(EXIT_FAILURE);
        }
    }
    return self->values[index];
}
void Env_insert_new(Env* const self, char* const name, Value value) {
    if (self->size == self->capacity) {
        self->capacity = self->capacity == 0 ? 1 : self->capacity * 2;
        self->names = (char**)realloc(self->names, sizeof(char*) * self->capacity);
        self->values = (Value*)realloc(self->values, sizeof(Value) * self->capacity);
    }
    size_t index = self->size++;
    self->names[index] = name;
    self->values[index] = value;
}
size_t Env_enter_scope(Env* const self) {
    return self->size;
}
void Env_leave_scope(Env* const self, size_t scope_marker) {
    assert(scope_marker <= self->size);
    self->size = scope_marker;
}

Value eval(Env* env, Node node) {
    size_t is_n_op = 0;
    switch (node.kind) {
        case ITEM_LIST: {
            Value result = (Value){
                .kind = VALUE_KIND_NUMBER,
                .number = 0,
            };
            for (size_t i = 0; i < node.data.children.size; ++i) {
                result = eval(env, node.data.children.data[i]);
            }
            return result;
        }
        case ITEM: return eval(env, node.data.children.data[0]);
        case EXPR: return eval(env, node.data.children.data[0]);
        case IF: {
            size_t children_size = node.data.children.size;
            assert(children_size == 2 || children_size == 3);
            Value cond = eval(env, node.data.children.data[0]);
            if (cond.kind != VALUE_KIND_NUMBER) {
                fprintf(stderr, "error: condition is not a number\n");
                exit(EXIT_FAILURE);
            }
            if (cond.number != 0) {
                return eval(env, node.data.children.data[1]);
            }
            return children_size == 3 ? eval(env, node.data.children.data[2]) : (Value){
                .kind = VALUE_KIND_NUMBER,
                .number = 0,
            };
        }
        case FUNCTION: {
            assert(node.data.children.size == 3);
            char* name = node.data.children.data[0].data.text;
            size_t args_size = node.data.children.data[1].data.children.size;
            Function function = {
                .args = (char**)malloc(sizeof(char*) * args_size),
                .args_size = args_size,
                .body = node.data.children.data[2].data.children,
            };
            for (size_t i = 0; i < args_size; ++i) {
                function.args[i] = node.data.children.data[1].data.children.data[i].data.text;
            }
            Value result = (Value) {
                .kind = VALUE_KIND_FUNC,
                .function = function,
            };
            Env_insert_new(env, name, result);
            return result;
       }
        case CONST_VAR: {
            assert(node.data.children.size == 2);
            char* name = node.data.children.data[0].data.text;
            Value value = eval(env, node.data.children.data[1]);
            Env_insert_new(env, name, value);
            return value;
        }
        case FUNC_CALL: {
            assert(node.data.children.size == 2);
            Value func = eval(env, node.data.children.data[0]);
            if (func.kind == VALUE_KIND_BUILTIN_FUNC) {
                switch (func.builtin_func) {
                    case BUILTIN_FUNC_PRINT: {
                        for (size_t i = 0; i < node.data.children.data[1].data.children.size; ++i) {
                            Value value = eval(env, node.data.children.data[1].data.children.data[i]);
                            if (value.kind == VALUE_KIND_NUMBER) {
                                Number number = value.number;
                                if (number == (int)number) {
                                    printf("%d\n", (int)number);
                                } else {
                                    printf("%f\n", number);
                                }
                            } else {
                                printf("<function>\n");
                            }
                        }
                        return (Value){ .kind = VALUE_KIND_NUMBER, .number = 0 };
                    }
                }
                assert(0);
            }
            if (func.kind != VALUE_KIND_FUNC) {
                fprintf(stderr, "error: %s is not a function\n", node.data.children.data[0].data.text);
                exit(EXIT_FAILURE);
            }
            Function function = func.function;
            size_t args_size = node.data.children.data[1].data.children.size;
            if (function.args_size != args_size) {
                fprintf(stderr, "error: argument count mismatch, expected %zu but got %zu\n", function.args_size, args_size);
                exit(EXIT_FAILURE);
            }
            size_t scope_marker = Env_enter_scope(env);
            for (size_t i = 0; i < function.args_size; ++i) {
                char* name = function.args[i];
                Value value = eval(env, node.data.children.data[1].data.children.data[i]);
                Env_insert_new(env, name, value);
            }
            Value result = (Value){
                .kind = VALUE_KIND_NUMBER,
                .number = 0,
            };
            for (size_t i = 0; i < function.body.size; ++i) {
                result = eval(env, function.body.data[i]);
            }
            Env_leave_scope(env, scope_marker);
            return result;
        }
        case VAR_REF: {
            char* name = node.data.text;
            return Env_get_value(env, name);
        }
        case NUMBER: return (Value){
            .kind = VALUE_KIND_NUMBER,
            .number = node.data.number,
        };
        case OR: {
            assert(node.data.children.size == 2);
            Value result = eval(env, node.data.children.data[0]);
            if (result.kind != VALUE_KIND_NUMBER) {
                fprintf(stderr, "error: lhs is not a number\n");
                exit(EXIT_FAILURE);
            }
            if (result.number == 0) {
                result = eval(env, node.data.children.data[1]);
            }
            return result;
        }
        case AND: {
            assert(node.data.children.size == 2);
            Value result = eval(env, node.data.children.data[0]);
            if (result.kind != VALUE_KIND_NUMBER) {
                fprintf(stderr, "error: lhs is not a number\n");
                exit(EXIT_FAILURE);
            }
            if (result.number != 0) {
                result = eval(env, node.data.children.data[1]);
            }
            return result;
        }
        case EQ:
        case NE:
        case LT:
        case GT:
        case LE:
        case GE:
        case ADD:
        case SUB:
        case MUL:
        case DIV:
            is_n_op = 2;
            break;
        case NOT:
        case NEG:
            is_n_op = 1;
            break;
        case ARG_LIST: fprintf(stderr, "bug: ARG_LIST should not be evaluated\n");
        case ARG: fprintf(stderr, "bug: ARG should not be evaluated\n");
        case NAME: fprintf(stderr, "bug: NAME should not be evaluated\n");
        default:
            assert(0);
    }
    assert(is_n_op == 1 || is_n_op == 2);
    assert(node.data.children.size == is_n_op);
    switch (is_n_op) {
        case 1: {
            Value result = eval(env, node.data.children.data[0]);
            if (result.kind != VALUE_KIND_NUMBER) {
                fprintf(stderr, "error: operand is not a number\n");
                exit(EXIT_FAILURE);
            }
            switch (node.kind) {
                case NOT: return (Value){
                    .kind = VALUE_KIND_NUMBER,
                    .number = (result.number == 0 ? 1 : 0),
                };
                case NEG: return (Value){
                    .kind = VALUE_KIND_NUMBER,
                    .number = -result.number,
                };
                default: assert(0);
            }
        }
        case 2: {
            Value lhs = eval(env, node.data.children.data[0]);
            Value rhs = eval(env, node.data.children.data[1]);
            if (lhs.kind != VALUE_KIND_NUMBER || rhs.kind != VALUE_KIND_NUMBER) {
                fprintf(stderr, "error: operands are not numbers\n");
                exit(EXIT_FAILURE);
            }
            double result;
            switch (node.kind) {
                case EQ:
                    result = (lhs.number == rhs.number ? 1 : 0);
                    break;
                case NE:
                    result = (lhs.number != rhs.number ? 1 : 0);
                    break;
                case LT:
                    result = (lhs.number < rhs.number ? 1 : 0);
                    break;
                case GT:
                    result = (lhs.number > rhs.number ? 1 : 0);
                    break;
                case LE:
                    result = (lhs.number <= rhs.number ? 1 : 0);
                    break;
                case GE:
                    result = (lhs.number >= rhs.number ? 1 : 0);
                    break;
                case ADD:
                    result = lhs.number + rhs.number;
                    break;
                case SUB:
                    result = lhs.number - rhs.number;
                    break;
                case MUL:
                    result = lhs.number * rhs.number;
                    break;
                case DIV:
                    if (rhs.number == 0) {
                        fprintf(stderr, "error: division by zero\n");
                        exit(EXIT_FAILURE);
                    }
                    result = lhs.number / rhs.number;
                    break;
                default: assert(0);
            }
            return (Value){
                .kind = VALUE_KIND_NUMBER,
                .number = result,
            };
        }
    }
    assert(0);
}

void start(Node root) {
    Env env = Env_new(2);
    eval(&env, root);
}
