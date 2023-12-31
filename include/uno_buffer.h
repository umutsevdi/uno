/******************************************************************************

 * File: include/memory.h
 *
 * Author: Umut Sevdi
 * Created: 06/25/23
 * Description: memory functions of the text editor

*****************************************************************************/

#ifndef __UNO_BUFFER__
#define __UNO_BUFFER__
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

typedef struct __UNO_BUFFER_LINE {
    wchar_t* str;
    uint64_t len;
    uint64_t cap;
    struct __UNO_BUFFER_LINE* prev;
    struct __UNO_BUFFER_LINE* next;
} UnoLine;

typedef struct __UNO_BUFFER {
    UnoLine* head;
    UnoLine* tail;
    UnoLine* current;
    uint64_t rows;
    uint64_t cursor_row;
    uint64_t cursor_col;
    wchar_t wchar_v;
} UnoBuffer;

/* Allocate a new line with no size. Free with {@link uno_line_destroy}. */
UnoLine* uno_line_new_no_buffer();

/* Allocate a new line. Free with {@link uno_line_destroy}. */
UnoLine* uno_line_new(uint64_t len);

/**
 * Grow or shrink given line the capacity of the line to the new size
 * * If the capacity is less then given new_size, new string is allocated
 * with the capacity of 2*(new_size+l->len)
 * @param l        - line to resize
 * @param new_size - new size
 */
void uno_line_resize(UnoLine* l, uint64_t new_size);

/**
 * Writes given string to the line. If the line is smaller than the
 * string, resizes the line.
 * @param l to write
 * @param str to insert
 * @param len length of the string
 */
void uno_line_write(UnoLine* l, const wchar_t* str, uint64_t len);

/**
 * Writes the given string to the end. If the line is smaller than
 * the string, resizes the line.
 * @param l to write
 * @param str to insert
 * @param len length of the string
 */
void uno_line_append(UnoLine* l, const wchar_t* str, uint64_t len);

/**
 * Writes the given string to the start. If the line is smaller than
 * the string, resizes the line.
 * @param l to write
 * @param str to insert
 * @param len length of the string
 */
void uno_line_prepend(UnoLine* l, const wchar_t* str, uint64_t len);

/* Frees the given line, along with the string it holds. */
void uno_line_destroy(UnoLine* l);

UnoBuffer* uno_buffer_new(uint64_t base_col);

void uno_buffer_add_line_head(UnoBuffer* b, UnoLine* l);

void uno_buffer_add_line_end(UnoBuffer* b, UnoLine* l);

void uno_buffer_add_line_to(UnoBuffer* b, UnoLine* l, uint64_t row);

void uno_buffer_swap(UnoBuffer* b, uint64_t r1, uint64_t r2);

UnoLine* uno_get_line_at(UnoBuffer* b, uint64_t row);

void uno_delete_line_at(UnoBuffer* b, uint64_t row);

void uno_buffer_destroy(UnoBuffer* b);

#endif // !__UNO_BUFFER__
