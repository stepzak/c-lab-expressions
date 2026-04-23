#ifndef STACK_H
#define STACK_H

typedef enum {
    STACK_OK,
    STACK_EMPTY,
    STACK_OVERFLOW,
    STACK_NULL
} StackStatus;

#include <stddef.h>
#include <stdlib.h>

#define DEFINE_STACK(postfix, T) \
    typedef struct Stack_##postfix { \
        T* data; \
        size_t len; \
        size_t capacity; \
    } Stack_##postfix; \
    \
    Stack_##postfix* init_stack_##postfix (size_t capacity); \
    StackStatus peek_stack_##postfix (Stack_##postfix* stack, T* output); \
    StackStatus pop_stack_##postfix (Stack_##postfix* stack, T* output); \
    StackStatus push_stack_##postfix (Stack_##postfix* stack, T value); \
    StackStatus len_stack_##postfix (Stack_##postfix* stack, size_t* output); \
    StackStatus capacity_stack_##postfix(Stack_##postfix* stack, size_t* output); \
    StackStatus is_empty_stack_##postfix(Stack_##postfix* stack, bool* output); \
    StackStatus is_full_stack_##postfix(Stack_##postfix* stack, bool* output); \
    StackStatus destroy_stack_##postfix (Stack_##postfix* stack); \
    StackStatus _expand_stack_##postfix (Stack_##postfix* stack, size_t new_capacity); \
    StackStatus resize_stack_##postfix (Stack_##postfix* stack, size_t new_len);

#define IMPLEMENT_STACK(postfix, T) \
    Stack_##postfix* init_stack_##postfix (size_t capacity) { \
        if (capacity == 0) capacity = 1; \
        Stack_##postfix* stack = (Stack_##postfix*) malloc(sizeof(Stack_##postfix)); \
        if (stack == NULL) return NULL; \
        stack->data = (T*) malloc(sizeof(T) * capacity); \
        if (stack->data == NULL) { \
            free(stack); \
            return NULL; \
        } \
        stack->len = 0; \
        stack->capacity = capacity; \
        return stack; \
    } \
    \
    StackStatus destroy_stack_##postfix (Stack_##postfix* stack) { \
        if (stack == NULL) return STACK_NULL; \
        free(stack->data); \
        free(stack); \
        return STACK_OK; \
    } \
    \
    StackStatus clear_stack_##postfix (Stack_##postfix* stack) { \
        if (stack == NULL) return STACK_NULL; \
        stack->len = 0; \
        return STACK_OK; \
    } \
    \
    StackStatus len_stack_##postfix(Stack_##postfix* stack, size_t* output) { \
        if (stack == NULL) return STACK_NULL; \
        if (output == NULL) return STACK_NULL; \
        *output = stack->len; \
        return STACK_OK; \
    } \
    \
    StackStatus capacity_stack_##postfix(Stack_##postfix* stack, size_t* output) { \
        if (stack == NULL) return STACK_NULL; \
        if (output == NULL) return STACK_NULL; \
        *output = stack->capacity; \
        return STACK_OK; \
    } \
    \
    StackStatus is_empty_stack_##postfix(Stack_##postfix* stack, bool* output) { \
        if (stack == NULL) return STACK_NULL; \
        if (output == NULL) return STACK_NULL; \
        *output = (stack->len == 0); \
        return STACK_OK; \
    } \
    \
    StackStatus is_full_stack_##postfix(Stack_##postfix* stack, bool* output) { \
        if (stack == NULL) return STACK_NULL; \
        if (output == NULL) return STACK_NULL; \
        *output = (stack->len == stack->capacity); \
        return STACK_OK; \
    } \
    \
    StackStatus peek_stack_##postfix (Stack_##postfix* stack, T* output) { \
        if (stack == NULL) return STACK_NULL; \
        if (output == NULL) return STACK_NULL; \
        if (stack->len == 0) return STACK_EMPTY; \
        *output = stack->data[stack->len - 1]; \
        return STACK_OK; \
    } \
    \
    StackStatus pop_stack_##postfix (Stack_##postfix* stack, T* output) { \
        if (stack == NULL) return STACK_NULL; \
        if (output == NULL) return STACK_NULL; \
        if (stack->len == 0) return STACK_EMPTY; \
        *output = stack->data[stack->len - 1]; \
        stack->len--; \
        return STACK_OK; \
    } \
    \
    StackStatus _expand_stack_##postfix (Stack_##postfix* stack, size_t new_capacity) { \
        if (stack == NULL) return STACK_NULL; \
        if (new_capacity <= stack->capacity) return STACK_OK; \
        T* new_data = (T*) realloc(stack->data, sizeof(T) * new_capacity); \
        if (new_data == NULL) return STACK_OVERFLOW; \
        stack->capacity = new_capacity; \
        stack->data = new_data; \
        return STACK_OK; \
    } \
    \
    StackStatus push_stack_##postfix(Stack_##postfix* stack, T value) { \
        if (stack == NULL) return STACK_NULL; \
        \
        if (stack->len >= stack->capacity) { \
            size_t new_capacity = (stack->capacity == 0) ? 8 : stack->capacity * 2; \
            StackStatus res = _expand_stack_##postfix(stack, new_capacity); \
            if (res != STACK_OK) return res; \
        } \
        \
        stack->data[stack->len++] = value; \
        return STACK_OK; \
    } \
    \
    StackStatus resize_stack_##postfix(Stack_##postfix* stack, size_t new_len) { \
        if (stack == NULL) return STACK_NULL; \
        \
        if (new_len > stack->capacity) { \
            StackStatus res = _expand_stack_##postfix(stack, new_len); \
            if (res != STACK_OK) return res; \
        } \
        stack->len = new_len; \
        return STACK_OK; \
    }
#endif //STACK_H