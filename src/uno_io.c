#include "uno_io.h"
#include "uno_buffer.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>

UnoBuffer* uno_io_file_to_buffer(FILE* f, size_t f_size);

UnoBuffer* uno_io_open(char* file, size_t file_l)
{
    FILE* f = fopen(file, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    size_t f_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return uno_io_file_to_buffer(f, f_size);
}

UnoBuffer* uno_io_file_to_buffer(FILE* f, size_t f_size)
{
    UnoBuffer* b = uno_buffer_new(0);
    wchar_t* f_buffer = calloc(f_size, sizeof(wchar_t));
    size_t b_read = fread(f_buffer, sizeof(wchar_t), f_size / sizeof(wchar_t), f);
    if (b_read != f_size / sizeof(wchar_t)) {
        free(f_buffer);
        fclose(f);
        return NULL;
    }

    size_t i = 0;
    size_t i_begin = 0;
    size_t i_end = 0;
    while (i < b_read) {
        if (f_buffer[i] == '\n' || f_buffer[i] == L'\n') {
            i_end = i;
            UnoLine* l = uno_line_new(i_end - i_begin + 1);
            uno_line_write(l, &f_buffer[i_begin], i_end);
            i_begin = i + 1;
            uno_buffer_add_line_end(b, l);
        }
        i++;
    }
    free(f_buffer);
    return b;
}
