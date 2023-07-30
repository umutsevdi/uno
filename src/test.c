#include "uno_buffer.h"
#include "uno_display.h"
#include "uno_movement.h"
#include <assert.h>
#include <locale.h>
#include <stdarg.h>
#include <wchar.h>

void uno_buffer_print(UnoBuffer* b)
{
    UnoLine* current = b->head;
    for (size_t i = 0; i < b->rows; i++) {
        wprintf(L"       (%2lu, %2lu) %ls\n", current->len, current->cap, current->str);
        current = current->next;
    }
}

#define TEST(s) \
    s();        \
    wprintf(L"[ OK ] " #s "\n")

#define BUFFER_DEF(b)                                 \
    UnoLine* l1 = uno_line_new(10);                   \
    uno_line_write(l1, L"Line 1", wcslen(L"Line 1")); \
    UnoLine* l2 = uno_line_new(10);                   \
    uno_line_write(l2, L"Line 2", wcslen(L"Line 2")); \
    UnoLine* l3 = uno_line_new(10);                   \
    uno_line_write(l3, L"Line 3", wcslen(L"Line 3")); \
    UnoBuffer* b = uno_buffer_new(1);                 \
    uno_buffer_add_line_head(b, l1);                  \
    uno_buffer_add_line_to(b, l2, 1);                 \
    uno_buffer_add_line_to(b, l3, 2);                 \
    b->current = l1;
#define ROW_DEF(b)            \
    size_t r = b->cursor_row; \
    size_t c = b->cursor_col;

void uno_move_empty_request_test()
{
    BUFFER_DEF(b);
    ROW_DEF(b);
    uno_move(b, 0);
    assert(b->cursor_row == r);
    assert(b->cursor_col == c);
}

void uno_move_backspace_test()
{
    BUFFER_DEF(b);
    ROW_DEF(b);
    b->cursor_col = 3, c = 3;
    assert(b->current != NULL);
    wchar_t* old_str = wcsdup(b->current->str);
    uno_move(b, UNO_RF_BACKSPACE);
    assert(b->cursor_row == r);
    assert(b->cursor_col == c - 1);
    assert(wcscmp(old_str, b->current->str));
    assert(!wcscmp(b->current->str, L"Lin 1"));
}

void uno_move_backspace_no_text_after_test()
{
    BUFFER_DEF(b);
    ROW_DEF(b);
    b->cursor_col = b->current->len, c = b->current->len;
    assert(b->current != NULL);
    wchar_t* old_str = wcsdup(b->current->str);
    uno_move(b, UNO_RF_BACKSPACE);
    uno_buffer_print(b);
    assert(b->cursor_row == r);
    assert(b->cursor_col == c - 1);
    wprintf(L"[%ls|%ls]\n", old_str, b->current->str);
    assert(wcscmp(old_str, b->current->str));
    assert(!wcscmp(b->current->str, L"Line "));
}

void uno_move_backspace_join_lines_test()
{
    BUFFER_DEF(b);
    ROW_DEF(b);
    b->current = l2;
    b->cursor_row = 1;
    b->cursor_col = 0;
    assert(b->current != NULL);
    wchar_t* old_str = wcsdup(b->current->str);
    wchar_t* old_pre_str = wcsdup(b->current->prev->str);
    uno_move(b, UNO_RF_BACKSPACE);

    assert(b->cursor_row == r);
    assert(b->current->len == wcslen(old_pre_str) + wcslen(old_str));
    assert(b->cursor_col == wcslen(old_pre_str));
    assert(!wcscmp(b->current->str, L"Line 1Line 2"));
}

void uno_move_delete_test()
{
    BUFFER_DEF(b);
    ROW_DEF(b);
    b->cursor_col = 3, c = 3;
    assert(b->current != NULL);
    wchar_t* old_str = wcsdup(b->current->str);
    uno_move(b, UNO_RF_SPEC_DEL);
    assert(b->cursor_row == r);
    assert(b->cursor_col == c);
    assert(wcscmp(old_str, b->current->str));
    assert(!wcscmp(b->current->str, L"Line1"));
}

void uno_move_NEWLINE_test()
{
    BUFFER_DEF(b);
    ROW_DEF(b);
    b->cursor_col = 3, c = 3;
    assert(b->current != NULL);
    wchar_t* old_str = wcsdup(b->current->str);
    uno_move(b, UNO_RF_SPEC_DEL);
    assert(b->cursor_row == r);
    assert(b->cursor_col == c);
    assert(wcscmp(old_str, b->current->str));
    assert(!wcscmp(b->current->str, L"Line1"));
}

void uno_line_get_line_at_test()
{
    BUFFER_DEF(buffer);
    UnoLine* to_compare = uno_get_line_at(buffer, 0);
    assert(to_compare == buffer->head);
    to_compare = uno_get_line_at(buffer, 1);
    assert(to_compare == buffer->head->next);
    to_compare = uno_get_line_at(buffer, 2);
    assert(to_compare == buffer->head->next->next);

    uno_buffer_destroy(buffer);
}

void uno_line_resize_test()
{
    UnoLine line1;
    line1.cap = 10;
    line1.len = 8;
    line1.str = wcsdup(L"abcdefghi");

    uno_line_resize(&line1, 5);
    assert(line1.cap == 10);
    assert(line1.len == 5);
    assert(wcscmp(line1.str, L"abcde") == 0);

    // Test expanding the line
    UnoLine line2;
    line2.cap = 5;
    line2.len = 3;
    line2.str = wcsdup(L"abc");

    uno_line_resize(&line2, 8);
    assert(line2.cap == 16);
    assert(line2.len == 3);
    assert(wcscmp(line2.str, L"abc") == 0);

    free(line1.str);
    free(line2.str);
}

void uno_line_write_test()
{
    // Test writing within the existing capacity
    UnoLine line1;
    line1.cap = 10;
    line1.len = 5;
    line1.str = calloc(line1.cap, sizeof(wchar_t));

    uno_line_write(&line1, L"Hello", 5);
    assert(line1.cap == 10);
    assert(line1.len == 5);
    assert(wcscmp(line1.str, L"Hello") == 0);

    // Test writing beyond the existing capacity
    UnoLine line2;
    line2.cap = 5;
    line2.len = 3;
    line2.str = calloc(line2.cap, sizeof(wchar_t));

    uno_line_write(&line2, L"Hello, World!", 13);
    assert(line2.cap == 26);
    assert(line2.len == 13);
    assert(wcscmp(line2.str, L"Hello, World!") == 0);

    free(line1.str);
    free(line2.str);
}

void uno_line_append_test()
{
    // Test appending within the existing capacity
    UnoLine line1;
    line1.cap = 10;
    line1.len = 5;
    line1.str = calloc(line1.cap, sizeof(wchar_t));
    wcpcpy(line1.str, L"Hello");

    uno_line_append(&line1, L", World!", 8);
    assert(line1.cap == 26);
    assert(line1.len == 13);
    assert(wcscmp(line1.str, L"Hello, World!") == 0);

    // Test appending beyond the existing capacity
    UnoLine line2;
    line2.cap = 5;
    line2.len = 3;
    line2.str = calloc(line2.cap, sizeof(wchar_t));
    wcpcpy(line2.str, L"abc");

    uno_line_append(&line2, L"defghijklmnopqrstuvwxyz", 23);
    assert(line2.cap == 52);
    assert(line2.len == 26);
    assert(wcscmp(line2.str, L"abcdefghijklmnopqrstuvwxyz") == 0);

    free(line1.str);
    free(line2.str);
}

void uno_line_prepend_test()
{
    // Test prepending within the existing capacity
    UnoLine line1;
    line1.cap = 10;
    line1.len = 6;
    line1.str = calloc(line1.cap, sizeof(wchar_t));
    wcpcpy(line1.str, L"World!");

    uno_line_prepend(&line1, L"Hello, ", 7);
    assert(line1.cap == 26);
    assert(line1.len == 13);
    assert(wcscmp(line1.str, L"Hello, World!") == 0);

    // Test prepending beyond the existing capacity
    UnoLine line2;
    line2.cap = 5;
    line2.len = 3;
    line2.str = calloc(line2.cap, sizeof(wchar_t));
    wcpcpy(line2.str, L"def");

    uno_line_prepend(&line2, L"abc", 3);
    assert(line2.cap == 12);
    assert(line2.len == 6);
    assert(wcscmp(line2.str, L"abcdef") == 0);

    free(line1.str);
    free(line2.str);
}

void uno_buffer_swap_when_head_and_other()
{
    UnoLine* line1 = uno_line_new(10);
    uno_line_write(line1, L"Line 1", wcslen(L"Line 1"));
    UnoLine* line2 = uno_line_new(10);
    uno_line_write(line2, L"Line 2", wcslen(L"Line 2"));

    UnoBuffer* buffer = uno_buffer_new(1);
    uno_buffer_add_line_head(buffer, line1);
    uno_buffer_add_line_to(buffer, line2, 1);

    uno_buffer_swap(buffer, 0, 1);

    assert(wcscmp(buffer->head->str, L"Line 2") == 0);
    assert(wcscmp(buffer->head->next->str, L"Line 1") == 0);

    uno_buffer_destroy(buffer);
}

void uno_buffer_swap_when_tail_and_other()
{
    UnoLine* line1 = uno_line_new(10);
    uno_line_write(line1, L"Line 1", wcslen(L"Line 1"));
    UnoLine* line2 = uno_line_new(10);
    uno_line_write(line2, L"Line 2", wcslen(L"Line 2"));

    UnoBuffer* buffer = uno_buffer_new(1);
    uno_buffer_add_line_to(buffer, line1, 1);
    uno_buffer_add_line_end(buffer, line2);

    uno_buffer_swap(buffer, 2, 1);

    assert(wcscmp(buffer->head->next->str, L"Line 2") == 0);
    assert(wcscmp(buffer->head->next->next->str, L"Line 1") == 0);

    uno_buffer_destroy(buffer);
}

void uno_buffer_swap_when_head_and_tail()
{
    UnoLine* line1 = uno_line_new(10);
    uno_line_write(line1, L"Line 1", wcslen(L"Line 1"));
    UnoLine* line2 = uno_line_new(10);
    uno_line_write(line2, L"Line 2", wcslen(L"Line 2"));
    UnoLine* line3 = uno_line_new(10);
    uno_line_write(line3, L"Line 3", wcslen(L"Line 3"));

    UnoBuffer* buffer = uno_buffer_new(1);
    uno_buffer_add_line_head(buffer, line1);
    uno_buffer_add_line_to(buffer, line3, 1);
    uno_buffer_add_line_end(buffer, line2);

    uno_buffer_swap(buffer, 0, 3);

    assert(wcscmp(buffer->head->str, L"Line 2") == 0);
    assert(wcscmp(buffer->tail->str, L"Line 1") == 0);

    uno_buffer_destroy(buffer);
}

void uno_buffer_swap_when_other_and_other()
{
    UnoLine* line1 = uno_line_new(10);
    uno_line_write(line1, L"Line 1", wcslen(L"Line 1"));
    UnoLine* line2 = uno_line_new(10);
    uno_line_write(line2, L"Line 2", wcslen(L"Line 2"));
    UnoLine* line3 = uno_line_new(10);
    uno_line_write(line3, L"Line 3", wcslen(L"Line 3"));
    UnoLine* line4 = uno_line_new(10);
    uno_line_write(line4, L"Line 4", wcslen(L"Line 4"));

    UnoBuffer* buffer = uno_buffer_new(1);
    uno_buffer_add_line_to(buffer, line1, 1);
    uno_buffer_add_line_to(buffer, line2, 2);
    uno_buffer_add_line_to(buffer, line3, 3);
    uno_buffer_add_line_to(buffer, line4, 4);

    uno_buffer_swap(buffer, 2, 3);

    assert(wcscmp(buffer->head->next->next->str, L"Line 3") == 0);
    assert(wcscmp(buffer->head->next->next->next->str, L"Line 2") == 0);

    uno_buffer_destroy(buffer);
}

void test_utf8()
{
    UnoLine* line1 = uno_line_new(10);
    uno_line_write(line1,
        L"Merhaba Dünya, İyi akşamlar!",
        wcslen(L"Merhaba Dünya, İyi akşamlar!"));
    UnoLine* line2 = uno_line_new(10);
    uno_line_write(line2, L"Hello world, good night!",
        wcslen(L"Hello world, good night!"));
    UnoLine* line3 = uno_line_new(10);
    uno_line_write(line3, L"Привет, мир, спокойной ночи !",
        wcslen(L"Привет, мир, спокойной ночи!"));
    UnoLine* line4 = uno_line_new(10);
    uno_line_write(line4, L"안녕 세상, 좋은 밤!",
        wcslen(L"안녕 세상, 좋은 밤!"));

    UnoBuffer* buffer = uno_buffer_new(1);
    uno_buffer_add_line_to(buffer, line1, 1);
    uno_buffer_add_line_to(buffer, line2, 2);
    uno_buffer_add_line_to(buffer, line3, 3);
    uno_buffer_add_line_to(buffer, line4, 4);
    uno_buffer_print(buffer);
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "en_US.UTF-8");
    wprintf(L"");
    TEST(uno_move_empty_request_test);
    TEST(uno_move_backspace_test);
    TEST(uno_move_backspace_no_text_after_test);
    TEST(uno_move_backspace_join_lines_test);
    TEST(uno_move_delete_test);
    TEST(uno_line_get_line_at_test);
    TEST(uno_line_resize_test);
    TEST(uno_line_write_test);
    TEST(uno_line_append_test);
    TEST(uno_line_prepend_test);

    TEST(uno_buffer_swap_when_head_and_other);
    TEST(uno_buffer_swap_when_tail_and_other);
    TEST(uno_buffer_swap_when_head_and_tail);
    TEST(uno_buffer_swap_when_other_and_other);

    TEST(test_utf8);

    return 0;
}
