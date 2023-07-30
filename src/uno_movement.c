#include "uno_movement.h"
#include "uno_buffer.h"
#include "uno_display.h"
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
    if (r == 0 && b->wchar_v == 0)
        return;
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
    else if (b->wchar_v != 0)
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
    UnoLine* nl = uno_line_new(UNO_BASE_SIZE);
    uno_buffer_add_line_to(b, nl, b->cursor_row + 1);
    if (b->cursor_col < b->current->len) {
        uno_line_write(nl, &b->current->str[b->cursor_col], b->current->len - b->cursor_col + 1);
        wmemset(&b->current->str[b->current->len], 0, b->current->len - b->cursor_col + 1);
        b->current->len = b->cursor_col;
    }
    b->current = b->current->next;
    b->cursor_row++;
    b->cursor_col = 0;
}

static void uno_move_bs(UnoBuffer* b)
{
    if (b->cursor_col != 0) {
        if (b->cursor_col < b->current->len) {
            // when a line has text after the cursor position
            // carry the text along with it
            wmemmove(&b->current->str[b->cursor_col],
                &b->current->str[b->cursor_col + 1],
                b->current->len - b->cursor_col);
            b->cursor_col--;
            b->current->len--;
        } else {
            // when at the end of line, just remove the character
            wprintf(L"THIS?\n");
            b->current->len--;
            b->current->str[b->current->len] = 0;
            b->cursor_col = b->current->len;
        }
    } else if (b->cursor_row != 0) {
        // when at the beginning of line, append current line
        // to the end of the previous line
        UnoLine* line_to_join = b->current->prev;
        size_t old_cursor = line_to_join->len;
        line_to_join->str[old_cursor] = ' ';
        uno_line_append(b->current->prev, b->current->str, b->current->len);
        uno_delete_line_at(b, b->cursor_row);
        b->cursor_row--;
        b->current = line_to_join;
        b->cursor_col = old_cursor;
    }
}

static void uno_move_del(UnoBuffer* b)
{
    if (b->cursor_col != b->current->len) {
        if (b->cursor_col < b->current->len) {
            wmemmove(&b->current->str[b->cursor_col + 1],
                &b->current->str[b->cursor_col + 2],
                b->current->len - b->cursor_col);
            b->current->len--;
        }
    } else {
        // when at the end of line, append current line
        // with the beginning of the next line
        UnoLine* line_to_join = b->current->next;
        size_t old_cursor = b->current->len;
        b->current->str[b->current->len] = ' ';
        uno_line_append(b->current, line_to_join->str, line_to_join->len);
        uno_delete_line_at(b, b->cursor_row + 1);
        b->cursor_col = old_cursor;
    }
}

static void uno_move_cmd(UnoBuffer* b, UnoRequest r)
{
    // TODO Add command feature
}
