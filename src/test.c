#include "uno_buffer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define START_TEST(s) printf("START: " #s "\n")
#define END_TEST(s) printf("PASS : " #s "\n")

void uno_line_resize_test()
{
    START_TEST(uno_line_resize_test);
    UnoLine line1;
    line1.cap = 10;
    line1.len = 8;
    line1.str = strdup("abcdefghi");

    uno_line_resize(&line1, 5);
    assert(line1.cap == 10);
    assert(line1.len == 5);
    assert(strcmp(line1.str, "abcde") == 0);

    // Test expanding the line
    UnoLine line2;
    line2.cap = 5;
    line2.len = 3;
    line2.str = strdup("abc");

    uno_line_resize(&line2, 8);
    assert(line2.cap == 8);
    assert(line2.len == 3);
    assert(strcmp(line2.str, "abc") == 0);

    free(line1.str);
    free(line2.str);
    END_TEST(uno_line_resize_test);
}

void uno_line_write_test()
{
    START_TEST(uno_line_write_test);

    // Test writing within the existing capacity
    UnoLine line1;
    line1.cap = 10;
    line1.len = 5;
    line1.str = calloc(line1.cap, sizeof(char));

    uno_line_write(&line1, "Hello", 5);
    assert(line1.cap == 10);
    assert(line1.len == 5);
    assert(strcmp(line1.str, "Hello") == 0);

    // Test writing beyond the existing capacity
    UnoLine line2;
    line2.cap = 5;
    line2.len = 3;
    line2.str = calloc(line2.cap, sizeof(char));

    uno_line_write(&line2, "Hello, World!", 13);
    assert(line2.cap == 13);
    assert(line2.len == 13);
    assert(strcmp(line2.str, "Hello, World!") == 0);

    free(line1.str);
    free(line2.str);

    END_TEST(uno_line_write_test);
}

void uno_line_append_test()
{
    START_TEST(uno_line_append_test);

    // Test appending within the existing capacity
    UnoLine line1;
    line1.cap = 10;
    line1.len = 5;
    line1.str = calloc(line1.cap, sizeof(char));
    strcpy(line1.str, "Hello");

    uno_line_append(&line1, ", World!", 8);
    assert(line1.cap == 13);
    assert(line1.len == 13);
    assert(strcmp(line1.str, "Hello, World!") == 0);

    // Test appending beyond the existing capacity
    UnoLine line2;
    line2.cap = 5;
    line2.len = 3;
    line2.str = calloc(line2.cap, sizeof(char));
    strcpy(line2.str, "abc");

    uno_line_append(&line2, "defghijklmnopqrstuvwxyz", 23);
    assert(line2.cap == 26);
    assert(line2.len == 26);
    assert(strcmp(line2.str, "abcdefghijklmnopqrstuvwxyz") == 0);

    free(line1.str);
    free(line2.str);

    END_TEST(uno_line_append_test);
}

void uno_line_prepend_test()
{
    START_TEST(uno_line_prepend_test);

    // Test prepending within the existing capacity
    UnoLine line1;
    line1.cap = 10;
    line1.len = 6;
    line1.str = calloc(line1.cap, sizeof(char));
    strcpy(line1.str, "World!");

    uno_line_prepend(&line1, "Hello, ", 7);
    assert(line1.cap == 13);
    assert(line1.len == 13);
    assert(strcmp(line1.str, "Hello, World!") == 0);

    // Test prepending beyond the existing capacity
    UnoLine line2;
    line2.cap = 5;
    line2.len = 3;
    line2.str = calloc(line2.cap, sizeof(char));
    strcpy(line2.str, "def");

    uno_line_prepend(&line2, "abc", 3);
    assert(line2.cap == 6);
    assert(line2.len == 6);
    assert(strcmp(line2.str, "abcdef") == 0);

    free(line1.str);
    free(line2.str);

    END_TEST(uno_line_prepend_test);
}

void uno_buffer_swap_when_head_and_other()
{
    START_TEST(uno_buffer_swap_when_head_and_other);
    UnoLine* line1 = uno_line_new(10);
    uno_line_write(line1, "Line 1", strlen("Line 1"));
    UnoLine* line2 = uno_line_new(10);
    uno_line_write(line2, "Line 2", strlen("Line 2"));

    UnoBuffer* buffer = uno_buffer_new(1);
    uno_buffer_add_line_head(buffer, line1);
    uno_buffer_add_line_to(buffer, line2, 1);

    uno_buffer_swap(buffer, 0, 1);

    assert(strcmp(buffer->head->str, "Line 2") == 0);
    assert(strcmp(buffer->head->next->str, "Line 1") == 0);

    uno_buffer_destroy(buffer);
    END_TEST(uno_buffer_swap_when_head_and_other);
}

void uno_buffer_swap_when_tail_and_other()
{
    START_TEST(uno_buffer_swap_when_tail_and_other);
    UnoLine* line1 = uno_line_new(10);
    uno_line_write(line1, "Line 1", strlen("Line 1"));
    UnoLine* line2 = uno_line_new(10);
    uno_line_write(line2, "Line 2", strlen("Line 2"));

    UnoBuffer* buffer = uno_buffer_new(1);
    uno_buffer_add_line_to(buffer, line1, 1);
    uno_buffer_add_line_end(buffer, line2);

    uno_buffer_swap(buffer, 2, 1);

    assert(strcmp(buffer->head->next->str, "Line 2") == 0);
    assert(strcmp(buffer->head->next->next->str, "Line 1") == 0);

    uno_buffer_destroy(buffer);
    END_TEST(uno_buffer_swap_when_tail_and_other);
}

void uno_buffer_swap_when_head_and_tail()
{
    START_TEST(uno_buffer_swap_when_head_and_tail);
    UnoLine* line1 = uno_line_new(10);
    uno_line_write(line1, "Line 1", strlen("Line 1"));
    UnoLine* line2 = uno_line_new(10);
    uno_line_write(line2, "Line 2", strlen("Line 2"));
    UnoLine* line3 = uno_line_new(10);
    uno_line_write(line3, "Line 3", strlen("Line 3"));

    UnoBuffer* buffer = uno_buffer_new(1);
    uno_buffer_add_line_head(buffer, line1);
    uno_buffer_add_line_to(buffer, line3, 1);
    uno_buffer_add_line_end(buffer, line2);

    uno_buffer_swap(buffer, 0, 3);

    assert(strcmp(buffer->head->str, "Line 2") == 0);
    assert(strcmp(buffer->tail->str, "Line 1") == 0);

    uno_buffer_destroy(buffer);
    END_TEST(uno_buffer_swap_when_head_and_tail);
}

void uno_buffer_swap_when_other_and_other()
{
    START_TEST(uno_buffer_swap_when_other_and_other);
    UnoLine* line1 = uno_line_new(10);
    uno_line_write(line1, "Line 1", strlen("Line 1"));
    UnoLine* line2 = uno_line_new(10);
    uno_line_write(line2, "Line 2", strlen("Line 2"));
    UnoLine* line3 = uno_line_new(10);
    uno_line_write(line3, "Line 3", strlen("Line 3"));
    UnoLine* line4 = uno_line_new(10);
    uno_line_write(line4, "Line 4", strlen("Line 4"));

    UnoBuffer* buffer = uno_buffer_new(1);
    uno_buffer_add_line_to(buffer, line1, 1);
    uno_buffer_add_line_to(buffer, line2, 2);
    uno_buffer_add_line_to(buffer, line3, 3);
    uno_buffer_add_line_to(buffer, line4, 4);

    uno_buffer_swap(buffer, 2, 3);

    assert(strcmp(buffer->head->next->next->str, "Line 3") == 0);
    assert(strcmp(buffer->head->next->next->next->str, "Line 2") == 0);

    uno_buffer_destroy(buffer);
    END_TEST(uno_buffer_swap_when_other_and_other);
}

int main(int argc, char* argv[])
{
    uno_line_resize_test();
    uno_line_write_test();
    uno_line_append_test();
    uno_line_prepend_test();

    uno_buffer_swap_when_head_and_other();
    uno_buffer_swap_when_tail_and_other();
    uno_buffer_swap_when_head_and_tail();
    uno_buffer_swap_when_other_and_other();

    return 0;
}
