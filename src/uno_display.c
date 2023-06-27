#include "uno_display.h"
#include "uno_buffer.h"
#define UNO_BASE_BUFFER_SIZE

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
    d->signal = UNO_SIG_READY;
    d->current_buffer = uno_buffer_new(80);
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
    FILE* f = popen("tput cols", "r");
    fgets(line, 16, f);
    t.col = atoi(line);
    pclose(f);
    f = popen("tput lines", "r");
    fgets(line, 16, f);
    t.row = atoi(line);
    pclose(f);
    return t;
}

void uno_fill_scr(UnoDisplay* d, Term t)
{
    UnoBuffer* b = d->current_buffer;
    char* line = calloc(t.col + 1, sizeof(char));
    FILE* fd_clear = popen("tput clear", "w");
    pclose(fd_clear);
    for (int i = 0; i < t.col; i++) {
        printf("-");
    }
    printf("\r\n");
    UnoLine* l = uno_get_line_at(b, b->cursor_row);
    sprintf(line, "%-40sr:%-4lu\tc%-4lu\ti:%c\tc:%c", "file.txt",
        b->cursor_row,
        b->cursor_col, d->c,
        l->str[b->cursor_col]);
    printf("%s\r\n", line);
    for (int i = 0; i < t.col; i++) {
        printf("-");
    }
    line[0] = '|';
    memset(line + 1, ' ', t.col - 1);
    line[t.col - 1] = '|';
    for (int i = 3; i < t.row - 2; i++) {
        printf("%s\r\n", line);
    }
    for (int i = 0; i < t.col; i++) {
        printf("-");
    }
    printf("\r\n");
    free(line);
    FILE* fd = popen("tput cup 3 0", "w");
    pclose(fd);
}

void uno_redraw(UnoDisplay* d)
{
    UnoBuffer* b = d->current_buffer;
    Term t = uno_get_terminal();
    uno_fill_scr(d, t);
    char* line = calloc(t.col + 1, sizeof(char));
    for (uint64_t i = 0; i < b->rows; i++) {
        UnoLine* l = uno_get_line_at(b, i);
        if (l == NULL) {
            continue;
        }
        strncpy(line, l->str, t.col);
        line[t.col - 1] = 0;
        if (l->len > t.col)
            line[t.col - 1] = '>';
        if (b->cursor_row == i) {
            uint64_t cursor = b->cursor_col < t.col - 2 ? b->cursor_col : t.col - 2;
            line[cursor] = '|';
        }
        printf("| %s\r\n", line);
    }
}
