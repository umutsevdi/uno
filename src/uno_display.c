#include "uno_display.h"
#include <stdio.h>

static void* display(void* sig)
{
    UnoDisplay* scr = (UnoDisplay*)sig;
    while (scr->signal != UNO_SIG_EXIT) {
        if (scr->signal) {
            uno_redraw(scr);
            scr->signal = UNO_SIG_READY;
        }
    }
    return NULL;
}

UnoDisplay* uno_display_start()
{
    UnoDisplay* d = malloc(sizeof(UnoDisplay));
    d->c = 0;
    d->signal = UNO_SIG_BUFFER;
    d->current_buffer = uno_buffer_new(UNO_BASE_SIZE);
    d->buffer = &d->current_buffer;
    d->buffer_count = 1;
    pthread_t thread;
    pthread_create(&thread, NULL, display, d);

    return d;
}

Term uno_get_terminal()
{
    Term t = { 0 };
    char line[16];
    char* endptr;
    FILE* f = popen("tput cols", "r");
    fgets(line, 16, f);
    pclose(f);
    t.col = strtoull(line, &endptr, 10);
    f = popen("tput lines", "r");
    fgets(line, 16, f);
    pclose(f);
    t.row = strtoull(line, &endptr, 10);
    return t;
}

void uno_fill_scr(UnoDisplay* d, Term t)
{
    UnoBuffer* b = d->current_buffer;

    FILE* fd_clear = popen("tput clear", "w");
    pclose(fd_clear);

    wchar_t* line = calloc(t.col + 1, sizeof(wchar_t));
    line[0] = L'┌';
    line[t.col - 1] = L'┐';
    wmemset(line + 1, L'─', t.col - 1);
    UNO_MULTISET(L'┬', wchar_t, 5,
        &line[43],
        &line[54],
        &line[65],
        &line[75],
        &line[84]);
    wprintf(L"%ls\r\n", line);
    UnoLine* l = uno_get_line_at(b, b->cursor_row);
    swprintf(line, t.col, L"│ %-40s │ ROW:%-4lu │ COL:%-4lu │ INPUT:%lc │ CHAR:%lc │", "file.txt",
        b->cursor_row,
        b->cursor_col, d->c != 0 ? d->c : L' ',
        l->str[b->cursor_col] != 0 ? l->str[b->cursor_col] : L' ');
    wprintf(L"%ls\r\n", line);

    line[0] = L'├';
    line[t.col - 1] = L'┤';
    wmemset(line + 1, L'─', t.col - 1);
    UNO_MULTISET(L'┴', wchar_t, 5,
        &line[43],
        &line[54],
        &line[65],
        &line[75],
        &line[84]);
    wprintf(L"%ls\r\n", line);

    line[0] = L'│';
    line[t.col - 1] = L'│';
    wmemset(line + 1, L' ', t.col - 1);
    for (size_t i = 3; i < t.row - 4; i++) {
        wprintf(L"%ls\r\n", line);
    }

    line[0] = L'└';
    line[t.col - 1] = L'┘';
    wmemset(line + 1, L'─', t.col - 1);
    wprintf(L"%ls\r\n", line);
    free(line);
    FILE* fd = popen("tput cup 3 0", "w");
    pclose(fd);
}

void uno_redraw(UnoDisplay* d)
{
//    return;
    UnoBuffer* b = d->current_buffer;
    Term t = uno_get_terminal();
     uno_fill_scr(d, t);
    wchar_t* line = calloc(t.col + 1, sizeof(wchar_t));
    for (size_t i = 0; i < b->rows; i++) {
        UnoLine* l = uno_get_line_at(b, i);
        if (l == NULL) {
            continue;
        }
        swprintf(line, t.col, L"%ls", l->str);
        line[t.col - 1] = 0;
        if (l->len > t.col)
            line[l->len] = L'╌';
        if (b->cursor_row == i) {
            uint64_t cursor = b->cursor_col < t.col - 2 ? b->cursor_col : t.col - 2;
            line[cursor] = L'█';
        }
        wprintf(L"│~%ls\r\n", line);
        wmemset(line, 0, t.col + 1);
    }
}
