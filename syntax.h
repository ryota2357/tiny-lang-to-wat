#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

typedef enum {
    // .children
    ITEM_LIST,
    ITEM,
    FUNCTION,
    CONST_VAR,
    FUNC_CALL,
    PARAM_LIST,
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

    PARAM,    // .text
    NAME,     // .text
    VAR_REF,  // .text
    NUMBER,   // .value
} SyntaxKind;

typedef double Number;

typedef struct {
    struct Node* data;
    size_t length;
    size_t _capacity;
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

static inline NodeArray NodeArray_new() {
    return (NodeArray){
        .data = NULL,
        .length = 0,
        ._capacity = 0,
    };
}

static inline void NodeArray_push_back(NodeArray* const self, const Node node) {
    if (self->length == self->_capacity) {
        self->_capacity = self->_capacity == 0 ? 1 : self->_capacity * 2;
        self->data = (Node*)realloc(self->data, sizeof(Node) * self->_capacity);
    }
    self->data[self->length++] = node;
};

static inline Node NodeArray_get(const NodeArray* const self, const size_t index) {
    assert(index < self->length);
    return self->data[index];
}

static inline Node Node_new(const SyntaxKind kind) {
    NodeData data;
    switch (kind) {
        case NAME:
        case PARAM:
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

static inline void Node_set_text(Node* const self, char* const text) {
    assert(self->kind == NAME || self->kind == PARAM || self->kind == VAR_REF);
    self->data.text = text;
}

static inline void Node_set_number(Node* const self, const Number number) {
    assert(self->kind == NUMBER);
    self->data.number = number;
}

static inline void Node_append_child(Node* const self, const Node child) {
    assert(self->kind != NAME && self->kind != NUMBER && self->kind != PARAM);
    NodeArray_push_back(&self->data.children, child);
}
