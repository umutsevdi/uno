#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <pthread.h>

char arr[10][256];
struct termios orig_termios;
void die(const char* s) { }
void stop_raw()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
    for (int i = 0; i < 10; i++) {
        printf("> %s\n", arr[i]);
    }
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

typedef struct {
    int row;
    int col;
} Term;

Term get_terminal()
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

enum DSIG {
    DSIG_EXIT = -1,
    DSIG_READY = 0,
    DSIG_BUFFER = 1
};

typedef struct {
    char c;
    enum DSIG signal;
    int row;
    int col;
} Screen;

void fill_scr(Screen* scr, Term t)
{
    char* line = calloc(t.col + 1, sizeof(char));
    FILE* fd_clear = popen("tput clear", "w");
    pclose(fd_clear);
    for (int i = 0; i < t.col; i++) {
        printf("-");
    }
    printf("\r\n");
    sprintf(line, "%-40sr:%-4d\tc%-4d\ti:%c\tc:%c", "file.txt", scr->row, scr->col, scr->c, arr[scr->row][scr->col]);
    printf("%s\r\n", line);
    for (int i = 0; i < t.col; i++) {
        printf("-");
    }
    line[0] = '|';
    for (int i = 1; i < t.col - 1; i++) {
        line[i] = ' ';
    }
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

void redraw(Screen* scr)
{
    Term t = get_terminal();
    fill_scr(scr, t);
    char* line = calloc(t.col + 1, sizeof(char));
    for (int i = 0; i < 10; i++) {
        strncpy(line, arr[i], t.col);
        line[t.col - 1] = 0;
        if (strlen(arr[i]) > t.col)
            line[t.col - 1] = '>';
        if (scr->row == i) {
            int cursor = scr->col < t.col - 2 ? scr->col : t.col - 2;
            line[cursor] = '|';
        }
        printf("| %s\r\n", line);
    }
}

void* display(void* sig)
{
    char cmd[255];
    Screen* scr = (Screen*)sig;
    while (scr->signal != DSIG_EXIT) {

        if (scr->signal) {
            redraw(scr);
            scr->signal = DSIG_READY;
        }
    }
    return NULL;
}

int main()
{
    popen("tput init", "w");
    pthread_t thread;
    Screen scr = { 0 };
    pthread_create(&thread, NULL, display, &scr);

    int i = 0, j = 0;
    int e_char = 0;
    int m_char = 0;
    begin_raw();
    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        /*        if (iscntrl(c)) {
                    printf("%d\r\n", c);
                } else */
        switch (c) {
        case 0:
            break;
        case 127:
            arr[i][j] = ' ';
            if (--j < 0) {
                j = 255;
                if (--i < 0) {
                    i = 0;
                    j = 0;
                }
            }
            e_char = 0;
            break;
        case '\r': // terminal mode new line or real new line
        case '\n':
            arr[i++][j + 1] = '\0';
            j = 0;
            e_char = 0;
            break;
        case '\e':
            e_char = !e_char;
            break;
        case '[': // if escape enabled, it should continue
            m_char = e_char;
            if (m_char)
                break;
        default:
            if (m_char) {
                switch (c) {
                case 'D':
                    // left
                    move_h(&i, &j, 0);
                    break;
                case 'B':
                    // down
                    i++;
                    break;
                case 'A':
                    i--;
                    break;
                    // up
                case 'C':
                    move_h(&i, &j, 1);
                    // right
                    break;
                }
                m_char = 0;
                e_char = 0;
                break;
            } else {
                e_char = 0;
                arr[i][j] = c;
                if (j < 256) {
                    j++;
                } else {
                    j = 0;
                    i++;
                }
                printf("%c\r\n", c);
            }
            break;
        }
        if (!e_char) {
            if (c == '\e') {
                scr.signal = DSIG_EXIT;
                break;
            }
        }
        if (c != '\0') {
            scr.signal = DSIG_BUFFER;
            scr.row = i;
            scr.col = j;
            scr.c = c;
        }
    }
    popen("tput clear", "w");
}
