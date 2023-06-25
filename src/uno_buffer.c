#include "uno_buffer.h"
#include <stdint.h>
#include <stdio.h>

UnoLine* uno_line_new(uint64_t len)
{
    char* str = calloc(len + 1, sizeof(char));
    printf("line\n");
    UnoLine* l = malloc(sizeof(UnoLine));
    l->str = str;
    l->cap = 2 * len;
    l->len = 0;
    l->prev = NULL;
    l->next = NULL;
    return l;
}

void uno_line_resize(UnoLine* l, uint64_t new_size)
{
    if (new_size < l->cap) {
        if (new_size < l->len)
            l->len = new_size;
        l->str[new_size] = '\0';
    } else {
        char* str = calloc(2 * new_size + 1, sizeof(char));
        memcpy(str, l->str, l->cap);
        free(l->str);
        l->str = str;
        l->str[new_size] = 0;
        l->cap = 2 * new_size;
    }
}

void uno_line_write(UnoLine* l, const char* str, uint64_t len)
{
    uint64_t to_cpy = len <= l->cap ? len : l->cap;
    printf("writing %lu char to \n", to_cpy);
    strncpy(l->str, str, to_cpy);
    l->len = to_cpy;
    if (str[len] != 0) {
        l->str[len] = 0;
    }
}

void uno_line_append(UnoLine* l, const char* str, uint64_t len)
{
    uint64_t char_left = l->cap - l->len;
    if (char_left < len) {
        uno_line_resize(l, len + l->len);
    }
    char_left = l->cap - l->len;

    uint64_t to_cpy = len <= char_left ? len : char_left;
    printf("copying %lu chars to \n", to_cpy);
    strncpy(&(l->str[l->len]), str, to_cpy);
    l->len = l->len + to_cpy;
}

void uno_line_prepend(UnoLine* l, const char* str, uint64_t len)
{
    uint64_t char_left = l->cap - l->len;
    uint64_t to_cpy = len <= char_left ? len : char_left;
    printf("copying %lu chars to \n", to_cpy);
    strncpy(&(l->str[l->len]), str, to_cpy);
}

void uno_line_destroy(UnoLine* l)
{
    free(l->str);
    free(l);
}

UnoBuffer* uno_buffer_new(uint64_t base_col)
{
    UnoBuffer* buffer = malloc(sizeof(UnoBuffer));
    UnoLine* head = uno_line_new(base_col);
    buffer->head = head;
    buffer->tail = head;
    buffer->rows = 1;
    return buffer;
}

void uno_buffer_add_line_head(UnoBuffer* b, UnoLine* l)
{
    l->next = b->head;
    l->prev = NULL;

    if (b->head != NULL) {
        b->head->prev = l;
    }

    b->head = l;

    if (b->tail == NULL) {
        b->tail = l;
    }
    b->rows++;
}

void uno_buffer_add_line_end(UnoBuffer* b, UnoLine* l)
{
    l->next = NULL;
    l->prev = b->tail;

    if (b->tail != NULL) {
        b->tail->next = l;
    }

    b->tail = l;

    if (b->head == NULL) {
        b->head = l;
    }
    b->rows++;
}

UnoBuffer* uno_buffer_add_line_to(UnoBuffer* b, UnoLine* l, uint64_t row)
{
    if (row == 0) {
        uno_buffer_add_line_head(b, l);
    } else if (row >= b->rows) {
        uno_buffer_add_line_end(b, l);
    } else {
        UnoLine* current = b->head;
        for (uint64_t i = 0; i < row; i++) {
            current = current->next;
        }

        l->prev = current->prev;
        l->next = current;

        if (current->prev != NULL) {
            current->prev->next = l;
        }

        current->prev = l;
        b->rows++;
    }

    return b;
}

void uno_delete_line_at(UnoBuffer* b, uint64_t row)
{
    if (row >= b->rows) {
        // Invalid row, nothing to delete
        return;
    }
    if (row == 0) {
        UnoLine* rm_line = b->head;
        b->head = rm_line->next;

        if (b->head != NULL) {
            b->head->prev = NULL;
        } else {
            // The buffer is now empty
            b->tail = NULL;
        }

        uno_line_destroy(rm_line);
    } else if (row == b->rows - 1) {
        UnoLine* rm_line = b->tail;
        b->tail = rm_line->prev;

        if (b->tail != NULL) {
            b->tail->next = NULL;
        } else {
            // The buffer is now empty
            b->head = NULL;
        }

        uno_line_destroy(rm_line);
    } else {
        UnoLine* current = b->head;

        for (uint64_t i = 0; i < row; i++) {
            current = current->next;
        }

        UnoLine* rm_line = current;
        current->prev->next = current->next;
        current->next->prev = current->prev;

        uno_line_destroy(rm_line);
    }

    b->rows--;
}

void uno_buffer_destroy(UnoBuffer* b)
{
    uint64_t rows = b->rows;
    UnoLine* current = b->head;
    for (uint64_t i = 0; i < b->rows; i++) {
        UnoLine* next = current->next;
        uno_line_destroy(current);
        current = next;
    }
    free(b);
}
