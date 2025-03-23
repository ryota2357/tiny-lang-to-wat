#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char* buffer;
    size_t length;
    size_t _capacity;
} StringBuilder;

static inline StringBuilder StringBuilder_new() {
    return (StringBuilder){
        .buffer = NULL,
        .length = 0,
        ._capacity = 0,
    };
};

static inline void StringBuilder_append(StringBuilder* const self, const char* text) {
    size_t len = strlen(text);
    if (self->length + len + 1 > self->_capacity) {
        size_t new_capacity = self->_capacity == 0 ? 1 : self->_capacity * 2;
        while (self->length + len + 1 > new_capacity) {
            new_capacity *= 2;
        }
        self->_capacity = new_capacity;
        self->buffer = (char*)realloc(self->buffer, sizeof(char) * self->_capacity);
    }
    memcpy(self->buffer + self->length, text, len);
    self->length += len;
    self->buffer[self->length] = '\0';
}

static inline void StringBuilder_appendf(StringBuilder* const self, const size_t buf_size, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char* buf = (char*)calloc(buf_size, sizeof(char));
    vsnprintf(buf, buf_size * sizeof(char), fmt, args);
    va_end(args);
    StringBuilder_append(self, buf);
}

static inline char* StringBuilder_to_string(const StringBuilder* const self) {
    char* text = (char*)malloc(self->length + 1);
    memcpy(text, self->buffer, self->length);
    text[self->length] = '\0';
    return text;
}

static inline void StringBuilder_clear(StringBuilder* const self) {
    free(self->buffer);
    self->buffer = NULL;
    self->length = 0;
    self->_capacity = 0;
}
