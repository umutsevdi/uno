#include "uno_buffer.h"
#include "uno_display.h"
#include "uno_movement.h"
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
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
    raw.c_iflag |= IUTF8;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

/** UTF8 characters have varying byte size. To read we have to read consecutively.
 * * If the first byte passes the bitmask then it's a UTF8 else ASCII
 *    - If ASCII return first byte as wchar_t.
 *    - Else read the mask
 *      - If the mask is bigger than single byte read the next byte for the mask
 *
 *  - Return remaining N characters, where N is found from the the mask.
 *  @return UTF-8 character
 */
int read_utf8(wchar_t* c)
{
    *c = 0;
    char buffer[6];
    ssize_t b_read = 0;

    // Read the first byte
    if (read(STDIN_FILENO, &buffer[0], 1) == -1) {
        perror("read");
        return 1;
        // corrupt UTF-8
    }

    // Determine the size of incoming UTF-8
    unsigned char b_init = buffer[0];
    unsigned char mask = 0x80;

    int b_count = 0;
    while ((b_init & mask) != 0) {
        b_count++;
        mask >>= 1;
    }

    // Read the remaining bytes
    if (b_count > 1) {
        b_read = read(STDIN_FILENO, &buffer[1], b_count - 1);
        if (b_read == -1) {
            perror("read");
            return 1;
            // corrupt UTF-8
        }
    } else {
        *c = buffer[0];
        return 0;
    }

    *c = (buffer[0] & (0xFF >> b_count));
    for (int i = 1; i < b_count; i++) {
        if ((buffer[i] & 0xC0) != 0x80) {
            // Invalid UTF-8 sequence.
            perror("read");
            return 2;
        }
        *c = (*c << 6) | (buffer[i] & 0x3F);
    }
    return 0;
}

int main()
{
    setlocale(LC_ALL, "en_US.UTF-8");
    popen("tput init", "w");
    begin_raw();
    UnoDisplay* d = uno_display_start();
    int i = 0;
    UnoRequest r = 0;
    while (1) {
        b = d->current_buffer;
        UnoLine* c_line = uno_get_line_at(d->current_buffer, i);
        if (c_line == NULL) {
            c_line = uno_line_new(80);
            uno_buffer_add_line_to(d->current_buffer, c_line, i);
        }
        wchar_t c;
        if (read_utf8(&c)) {
            continue;
        }

        switch (c) {
        case 0:
            break;
        case '\x7f': // backspace
            uno_move(b, r | UNO_RF_BACKSPACE);
            break;
            //        case L'\r': // terminal mode new line or real new line
        case L'\r':
        case L'\n':
            uno_move(b, r | UNO_RF_NEWLINE);
            break;
        case L'\e':
            if (r & UNO_RF_ESCAPE)
                r = 0;
            else
                r |= UNO_RF_ESCAPE;
            break;
        case L'[': // if escape enabled, it should continue
            if ((r & UNO_RF_ESCAPE) == UNO_RF_ESCAPE)
                r = UNO_RF_DIR_ENABLED;
            if (r & UNO_RF_DIR_ENABLED) {
                break;
            }
        default:
            // If DIR enabled its either movement char
            // or SPEC character
            if (r & UNO_RF_DIR_ENABLED) {
                switch (c) {
                case UNO_RF_WCHAR_L:
                    r |= UNO_RF_DIR_LEFT;
                    break;
                case UNO_RF_WCHAR_D:
                    r |= UNO_RF_DIR_DOWN;
                    break;
                case UNO_RF_WCHAR_U:
                    r |= UNO_RF_DIR_UP;
                    break;
                case UNO_RF_WCHAR_R:
                    r |= UNO_RF_DIR_RIGHT;
                    break;
                // Overwrite characters
                // These characters leave '~' to be rendered
                case UNO_RF_WCHAR_DEL:
                    r = UNO_RF_SPEC_DEL;
                    break;
                case UNO_RF_WCHAR_INS:
                    r = UNO_RF_SPEC_INS;
                    break;
                case UNO_RF_WCHAR_PGUP:
                    r = UNO_RF_SPEC_PGUP;
                    break;
                case UNO_RF_WCHAR_PGDOWN:
                    r = UNO_RF_SPEC_PGDW;
                    break;
                }
                uno_move(b, r);
                // Discard that extra character if SPEC
                if (r & UNO_RF_SPEC_MASK)
                    read_utf8(&c);
                r = 0;
                break;
            } else {
                r = 0;
                b->wchar_v = c;
                uno_move(b, r);
            }
            break;
        }
        if (!(r & UNO_RF_ESCAPE)) {
            if (c == '\e') {
                d->signal = UNO_SIG_EXIT;
                break;
            }
        }
        if (c != '\0') {
            d->signal = UNO_SIG_BUFFER;
        }
    }

    popen("tput clear", "w");
}
