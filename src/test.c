#include "uno_buffer.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
    UnoLine* l = uno_line_new(10);
    char* c = "hello world this is a long text";
    uno_line_write(l, c, strlen(c));
    printf("[%s]{%lu %lu}\n", l->str, l->len, l->cap);
    uno_line_resize(l, 5);
    printf("[%s]{%lu %lu}\n", l->str, l->len, l->cap);
    uno_line_resize(l, 20);
    printf("[%s]{%lu %lu}\n", l->str, l->len, l->cap);
    uno_line_append(l, c, strlen(c));
    printf("[%s]{%lu %lu}\n", l->str, l->len, l->cap);
    printf("SAFE\n");
    char* c2 = "Hello, Zuko here!";
    UnoLine* l_safe = uno_line_new(20);
    uno_line_write(l_safe, c2, strlen(c2));
    printf("[%s]{%lu %lu}\n", l_safe->str, l_safe->len, l_safe->cap);
    uno_line_resize(l_safe, 5);
    printf("[%s]{%lu %lu}\n", l_safe->str, l_safe->len, l_safe->cap);
    uno_line_resize(l_safe, 20);
    printf("[%s]{%lu %lu}\n", l_safe->str, l_safe->len, l_safe->cap);
    uno_line_append(l_safe, c2, strlen(c2));
    printf("[%s]{%lu %lu}\n", l_safe->str, l_safe->len, l_safe->cap);
    printf("endoflinetest\n");

    UnoBuffer* b = uno_buffer_new(1);
    uno_buffer_add_line_head(b, l);
    uno_buffer_add_line_to(b, l_safe, 2);

    printf("Buffer(%lu):[\n%s\n%s\n%s]\n", b->rows, b->head->str, b->head->next->str, b->head->next->next->str);
    uno_delete_line_at(b, 2);

    printf("Buffer(%lu):\n%s\n%s\n%s\n", b->rows, b->head->str, b->head->next->str, b->head->next->next->str);

    uno_line_destroy(l);
    uno_line_destroy(l_safe);
}
