#include "uno_movement.h"
#include "uno_buffer.h"
#include <string.h>
#include <wchar.h>

static void uno_move_arrow(UnoBuffer* b, UnoRequest r);
static void uno_move_wchar(UnoBuffer* b);
static void uno_move_nl(UnoBuffer* b);
static void uno_move_bs(UnoBuffer* b);
static void uno_move_del(UnoBuffer* b);
static void uno_move_cmd(UnoBuffer* b, UnoRequest r);

void uno_move(UnoBuffer* b, UnoRequest r)
{
    if (b->current == NULL)
        b->current = uno_get_line_at(b, b->cursor_row);
    // Only move when E_CHAR and M_CHAR enabled
    if (UNO_RF_EQ(r, UNO_RF_DIR_ENABLED))
        uno_move_arrow(b, r);
    else if (UNO_RF_EQ(r, UNO_RF_BACKSPACE))
        uno_move_bs(b);
    else if (UNO_RF_EQ(r, UNO_RF_SPEC_DEL))
        uno_move_del(b);
    else if (UNO_RF_EQ(r, UNO_RF_NEWLINE))
        uno_move_nl(b);
    else if (UNO_RF_EQ(r, UNO_RF_CMD_ENABLED))
        uno_move_cmd(b, r);
    else
        uno_move_wchar(b);
}

static void uno_move_arrow(UnoBuffer* b, UnoRequest r)
{
    switch (r & UNO_RF_DIR_MASK) {
    case UNO_RF_DIR_LEFT:
        if (b->cursor_col != 0) {
            b->cursor_col--;
        } else if (b->cursor_row != 0) {
            b->cursor_row--;
            b->current = b->current->prev;
        }
        break;
    case UNO_RF_DIR_DOWN:
        if (b->current == b->tail) {
            uno_buffer_add_line_end(b, uno_line_new(UNO_BASE_SIZE));
        }
        b->cursor_row++;
        b->current = b->current->next;
        if (b->cursor_col > b->current->len)
            b->cursor_col = b->current->len;
        break;
    case UNO_RF_DIR_UP:
        if (b->cursor_row > 0) {
            b->cursor_row--;
            b->current = b->current->prev;
            if (b->cursor_col > b->current->len)
                b->cursor_col = b->current->len;
        }
        break;
    case UNO_RF_DIR_RIGHT:
        if (b->current->str[b->cursor_col] == 0) {
            b->current->str[b->cursor_col] = ' ';
            b->current->len++;
        }
        b->cursor_col++;
        if (b->cursor_col >= b->current->len) {
            /* Double the cap if not enough */
            if (b->cursor_col >= b->current->cap)
                uno_line_resize(b->current, b->current->cap);
            b->current->str[b->cursor_col] = ' ';
            b->current->len++;
        }
        break;
    }
}

static void uno_move_wchar(UnoBuffer* b)
{
    /* Double the cap if not enough */
    if (b->cursor_col >= b->current->cap)
        uno_line_resize(b->current, b->current->cap);
    if (b->cursor_col < b->current->len)
        wmemmove(&b->current->str[b->cursor_col + 1],
            &b->current->str[b->cursor_col],
            b->current->len - b->cursor_col);
    b->current->str[b->cursor_col] = b->wchar_v;
    b->cursor_col++;
    b->current->len++;
    b->wchar_v = L' ';
}

static void uno_move_nl(UnoBuffer* b)
{
    if (b->current == b->tail)
        uno_buffer_add_line_end(b, uno_line_new(UNO_BASE_SIZE));
    b->current = b->current->next;
    b->cursor_row++;
    b->cursor_col = 0;
    b->current->str[b->cursor_col] = ' ';
}

static void uno_move_bs(UnoBuffer* b)
{
    if (b->cursor_col != 0) {
        if (b->cursor_col < b->current->len)
            wmemmove(&b->current->str[b->cursor_col],
                &b->current->str[b->cursor_col + 1],
                b->current->len - b->cursor_col);
        b->cursor_col--;
        b->current->len--;
    } else if (b->cursor_row != 0) {
        // TODO Join current line to the previous line
        b->cursor_row--;
        b->current = b->current->prev;
    }
}

static void uno_move_del(UnoBuffer* b)
{
    if (b->current->len > 0) {
        if (b->cursor_col < b->current->len) {
            wmemmove(&b->current->str[b->cursor_col + 1],
                &b->current->str[b->cursor_col + 2],
                b->current->len - b->cursor_col);
            b->current->len--;
        }
    } else {
        // TODO Join the next line to the current
    }
}

static void uno_move_cmd(UnoBuffer* b, UnoRequest r)
{
    // TODO Add command feature
}
