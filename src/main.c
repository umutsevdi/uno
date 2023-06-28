#include "uno_buffer.h"
#include "uno_display.h"
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <wchar.h>

UnoBuffer* b;
struct termios orig_termios;
void die(const char* s) { }
void stop_raw()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
    uno_buffer_print(b);
}

void begin_raw()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    atexit(stop_raw);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

void move_h(int* i, int* j, int dir)
{
    if (dir) {
        if (++(*j) > 255) {
            j = 0;
            if (++(*i) > 10) {
                *i = 9;
                *j = 255;
            }
        }

    } else {
        if (--(*j) < 0) {
            *j = 255;
            if (--(*i) < 0) {
                *i = 0;
                *j = 0;
            }
        }
    }
}

int main()
{
    setlocale(LC_ALL, "en_US.UTF-8");
    popen("tput init", "w");
    UnoDisplay* d = uno_display_start();

    int i = 0, j = 0;
    int e_char = 0;
    int m_char = 0;
    begin_raw();
    while (1) {
        b = d->current_buffer;
        UnoLine* c_line = uno_get_line_at(d->current_buffer, i);
        if (c_line == NULL) {
            c_line = uno_line_new(80);
            uno_buffer_add_line_to(d->current_buffer, c_line, i);
        }
        wchar_t c = L'\0';
        if (read(STDIN_FILENO, &c, sizeof(wchar_t)) == -1 && errno != EAGAIN)
            die("read");
        switch (c) {
        case 0:
            break;
        case 127:
            c_line->str[j] = ' ';
            if (--j < 0) {
                j = 255;
                if (--i < 0) {
                    i = 0;
                    j = 0;
                }
            }
            e_char = 0;
            break;
        case L'\r': // terminal mode new line or real new line
        case L'\n':
            i++;
            c_line->str[j + 1] = L'\0';
            j = 0;
            e_char = 0;
            break;
        case L'\e':
            e_char = !e_char;
            break;
        case L'[': // if escape enabled, it should continue
            m_char = e_char;
            if (m_char)
                break;
        default:
            if (m_char) {
                switch (c) {
                case L'D':
                    // left
                    move_h(&i, &j, 0);
                    break;
                case L'B':
                    // down
                    i++;
                    break;
                case L'A':
                    i--;
                    break;
                    // up
                case L'C':
                    move_h(&i, &j, 1);
                    // right
                    break;
                }
                m_char = 0;
                e_char = 0;
                break;
            } else {
                e_char = 0;
                c_line->str[j] = c;
                if (j < 256) {
                    j++;
                } else {
                    j = 0;
                    i++;
                }
                //                wprintf(L"[%lc]\r\n", c);
                //                wprintf(L"%lc\r\n", c);
            }
            break;
        }
        if (!e_char) {
            if (c == '\e') {
                d->signal = UNO_SIG_EXIT;
                break;
            }
        }
        if (c != '\0') {
            d->signal = UNO_SIG_BUFFER;
            d->c = c;
            d->current_buffer->cursor_row = i;
            d->current_buffer->cursor_col = j;
        }
    }
    popen("tput clear", "w");
}
