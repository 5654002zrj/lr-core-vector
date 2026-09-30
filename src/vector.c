/* vector.c —— 你要实现的地方 */

#include "vector.h"

int vector_init(vector *v, size_t capacity) {
    return 0;
}

void vector_destroy(vector *v) {}

size_t size(const vector *v) {
    return 0;
}

size_t capacity(const vector *v) {
    return 0;
}

int empty(const vector *v) {
    return 0;
}

int get(const vector *v, size_t index, int *out) {
    return 0;
}

int set(vector *v, size_t index, int value) {
    return 0;
}

int front(const vector *v, int *out) {
    return 0;
}

int back(const vector *v, int *out) {
    return 0;
}

int push_back(vector *v, int value) {
    return 0;
}

int pop_back(vector *v, int *out) {
    return 0;
}

int reserve(vector *v, size_t new_capacity) {
    return 0;
}

int shrink_to_fit(vector *v) {
    return 0;
}

void clear(vector *v) {}
