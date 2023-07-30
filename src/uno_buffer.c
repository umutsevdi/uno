#include "uno_buffer.h"

#define UNO_EOL(l)               \
    if (l->str[l->len + 1] != 0) \
        l->str[l->len + 1] = 0;

UnoLine* uno_line_new_no_buffer()
{
    return calloc(1, sizeof(UnoLine));
}

UnoLine* uno_line_new(uint64_t len)
{
    wchar_t* str = calloc(len + 1, sizeof(wchar_t));
    UnoLine* l = malloc(sizeof(UnoLine));
    l->str = str;
    l->cap = len;
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
        l->str[new_size] = 0;
    } else {
        wchar_t* str = calloc(2 * new_size + 1, sizeof(wchar_t));
        wmemcpy(str, l->str, l->cap);
        free(l->str);
        l->str = str;
        l->str[new_size] = 0;
        l->cap = 2 * new_size;
    }
}

void uno_line_write(UnoLine* l, const wchar_t* str, uint64_t len)
{
    if (l->cap < len) {
        uno_line_resize(l, len);
    }
    wmemcpy(l->str, str, len);
    l->len = len;
    UNO_EOL(l);
}

void uno_line_append(UnoLine* l, const wchar_t* str, uint64_t len)
{
    size_t char_left = l->cap - l->len;
    if (char_left < len) {
        uno_line_resize(l, len + l->len);
    }
    char_left = l->cap - l->len;
    uint64_t to_cpy = len <= char_left ? len : char_left;
    wmemcpy(l->str + l->len, str, to_cpy);
    l->len = l->len + to_cpy;
    UNO_EOL(l);
}

void uno_line_prepend(UnoLine* l, const wchar_t* str, uint64_t len)
{
    size_t char_left = l->cap - l->len;
    if (char_left < len) {
        uno_line_resize(l, len + l->len);
    }
    wmemmove(l->str + len, l->str, l->len);
    wmemcpy(l->str, str, len);
    l->len = l->len + len;
    UNO_EOL(l);
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

void uno_buffer_add_line_to(UnoBuffer* b, UnoLine* l, uint64_t row)
{
    if (row == 0) {
        uno_buffer_add_line_head(b, l);
    } else if (row >= b->rows) {
        uno_buffer_add_line_end(b, l);
    } else {
        UnoLine* current = b->head;
        for (size_t i = 0; i < row; i++) {
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
}

void uno_buffer_swap(UnoBuffer* b, uint64_t r1, uint64_t r2)
{
    if (b->rows <= r1 || b->rows <= r2 || r1 == r2) {
        return;
    }
    UnoLine* current = b->head;
    UnoLine* row_ptr1 = NULL;
    UnoLine* row_ptr2 = NULL;
    uint64_t row_iter = 0;

    while (row_iter < b->rows && (row_ptr1 == NULL || row_ptr2 == NULL)) {
        if (r1 == row_iter)
            row_ptr1 = current;
        else if (r2 == row_iter)
            row_ptr2 = current;
        current = current->next;
        row_iter++;
    }
    if (row_ptr1 == NULL || row_ptr2 == NULL)
        return; // shouldn't occur
    wchar_t* str = row_ptr1->str;
    size_t len = row_ptr1->len;
    size_t cap = row_ptr1->cap;
    row_ptr1->str = row_ptr2->str;
    row_ptr1->len = row_ptr2->len;
    row_ptr1->cap = row_ptr2->cap;
    row_ptr2->str = str;
    row_ptr2->len = len;
    row_ptr2->cap = cap;
}

UnoLine* uno_get_line_at(UnoBuffer* b, uint64_t row)
{
    if (b->rows < row) {
        return NULL;
    }

    UnoLine* c = b->head;
    for (size_t i = 0; i < row; i++) {
        c = c->next;
    }
    return c;
}

void uno_delete_line_at(UnoBuffer* b, uint64_t row)
{
    if (row >= b->rows) {
        return;
    }
    if (row == 0) {
        UnoLine* rm_line = b->head;
        b->head = rm_line->next;

        if (b->head != NULL) {
            b->head->prev = NULL;
        } else {
            b->tail = NULL; // The buffer is now empty
        }

        uno_line_destroy(rm_line);
    } else if (row == b->rows - 1) {
        UnoLine* rm_line = b->tail;
        b->tail = rm_line->prev;
        if (b->tail != NULL) {
            b->tail->next = NULL;
        } else {
            b->head = NULL; // The buffer is now empty
        }
        uno_line_destroy(rm_line);
    } else {
        UnoLine* current = b->head;

        for (size_t i = 0; i < row; i++)
            current = current->next;

        UnoLine* rm_line = current;
        current->prev->next = current->next;
        current->next->prev = current->prev;

        uno_line_destroy(rm_line);
    }
    b->rows--;
}

void uno_buffer_destroy(UnoBuffer* b)
{
    UnoLine* current = b->head;
    for (size_t i = 0; i < b->rows; i++) {
        UnoLine* next = current->next;
        uno_line_destroy(current);
        current = next;
    }
    free(b);
}
