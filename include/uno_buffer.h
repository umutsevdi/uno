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

typedef struct __UNO_BUFFER_LINE {
    char* str;
    uint64_t len;
    uint64_t cap;
    struct __UNO_BUFFER_LINE* prev;
    struct __UNO_BUFFER_LINE* next;
} UnoLine;

typedef struct {
    UnoLine* head;
    UnoLine* tail;
    uint64_t rows;
} UnoBuffer;

/* Allocate a new line. Free with {@link uno_line_destroy}. */
UnoLine* uno_line_new(uint64_t len);

/**
 * Grow or shrink given line the capacity of the line to the new size
 * @l        - line to resize
 * @new_size - new size
 */
void uno_line_resize(UnoLine* l, uint64_t new_size);

void uno_line_write(UnoLine* l, const char* str, uint64_t len);

void uno_line_append(UnoLine* l, const char* str, uint64_t len);
/* Frees the given line, along with the string it holds. */
void uno_line_destroy(UnoLine* l);

UnoBuffer* uno_buffer_new(uint64_t base_col);

void uno_buffer_add_line_head(UnoBuffer* b, UnoLine* l);

void uno_buffer_add_line_end(UnoBuffer* b, UnoLine* l);

UnoBuffer* uno_buffer_add_line_to(UnoBuffer* b, UnoLine* l, uint64_t row);

void uno_delete_line_at(UnoBuffer* b, uint64_t row);

void uno_buffer_destroy(UnoBuffer* b);

#endif // !__UNO_BUFFER__
