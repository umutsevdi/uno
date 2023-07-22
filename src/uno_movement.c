#include "uno_movement.h"
#include "uno_buffer.h"

static void uno_move_cursor_arrow(UnoBuffer* b, UnoRequest r);

void uno_move_cursor(UnoBuffer* b, UnoRequest r)
{
    if (b->current == NULL)
        b->current = uno_get_line_at(b, b->cursor_row);
    // Only move when E_CHAR and M_CHAR enabled
    if (UNO_NTH_HEX(r, 7) & ~(E_CHAR | M_ENABLED | 0xF0)) {
        return;
    }
    switch (r & UNO_DIR_MASK) {
    case UNO_DIR_LEFT:
        if (b->cursor_col != 0) {
            b->cursor_col--;
        } else if (b->cursor_row != 0) {
            b->cursor_row--;
            b->current = b->current->prev;
        }
        break;
    case UNO_DIR_DOWN:
        if (b->current != b->tail) {
            b->cursor_row++;
            b->current = b->current->next;
            if (b->cursor_col > b->current->len)
                b->cursor_col = b->current->len;
        } else {
            uno_buffer_add_line_end(b, uno_line_new(UNO_BASE_SIZE));
        }
        break;
    case UNO_DIR_UP:
        if (b->cursor_row != 0) {
            b->cursor_row--;
            b->current = b->current->prev;
            if (b->cursor_col > b->current->len)
                b->cursor_col = b->current->len;
        } else
            b->cursor_col = 0;
        break;
    case UNO_DIR_RIGHT:
        if (b->current->str[b->cursor_col] == 0) {
            b->current->str[b->cursor_col] = ' ';
        }
        b->cursor_col++;
        if (b->cursor_col >= b->current->len) {
            /* Double the cap if not enough */
            if (b->cursor_col >= b->current->cap)
                uno_line_resize(b->current, b->current->cap);
            b->current->str[b->cursor_col] = ' ';
        }
        b->current->len++;
        break;
    }
}

static void uno_move_cursor_arrow(UnoBuffer* b, UnoRequest r)
{
    switch (r & UNO_DIR_MASK) {
    case UNO_DIR_LEFT:
        if (b->cursor_col != 0) {
            b->cursor_col--;
        } else if (b->cursor_row != 0) {
            b->cursor_row--;
            b->current = b->current->prev;
        }
        break;
    case UNO_DIR_DOWN:
        if (b->current != b->tail) {
            b->cursor_row++;
            b->current = b->current->next;
            if (b->cursor_col > b->current->len)
                b->cursor_col = b->current->len;
        } else {
            uno_buffer_add_line_end(b, uno_line_new(UNO_BASE_SIZE));
        }
        break;
    case UNO_DIR_UP:
        if (b->cursor_row != 0) {
            b->cursor_row--;
            b->current = b->current->prev;
            if (b->cursor_col > b->current->len)
                b->cursor_col = b->current->len;
        } else
            b->cursor_col = 0;
        break;
    case UNO_DIR_RIGHT:
        if (b->current->str[b->cursor_col] == 0) {
            b->current->str[b->cursor_col] = ' ';
        }
        b->cursor_col++;
        if (b->cursor_col >= b->current->len) {
            /* Double the cap if not enough */
            if (b->cursor_col >= b->current->cap)
                uno_line_resize(b->current, b->current->cap);
            b->current->str[b->cursor_col] = ' ';
        }
        b->current->len++;
        break;
    }
}
