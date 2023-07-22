/******************************************************************************

 * File: include/uno_display.h
 *
 * Author: Umut Sevdi
 * Created: 06/27/23
 * Description: Display utilities

*****************************************************************************/
#ifndef __UNO_DISP__
#define __UNO_DISP__
#include "uno_buffer.h"

#include <pthread.h>
#include <unistd.h>

#define UNO_BASE_SIZE 80

// Set __SIZE elements of type __TYPE to the _VALUE
#define UNO_MULTISET(_VALUE, _TYPE, _SIZE, ...) \
    do {                                        \
        _TYPE* _ARRAY[] = { __VA_ARGS__ };      \
        for (size_t _i = 0; _i < _SIZE; ++_i)   \
            *_ARRAY[_i] = _VALUE;               \
    } while (0);

typedef enum DSIG {
    UNO_SIG_EXIT = -1,
    UNO_SIG_READY = 0,
    UNO_SIG_BUFFER = 1
} UNO_SIG;

typedef struct {
    uint64_t row;
    uint64_t col;
} Term;

typedef struct _UNO_DISP {
    wchar_t c;
    enum DSIG signal;
    UnoBuffer* current_buffer;
    UnoBuffer** buffer;
    uint64_t buffer_count;
} UnoDisplay;

Term uno_get_terminal();

UnoDisplay* uno_display_start();

void uno_redraw(UnoDisplay* d);

void uno_fill_scr(UnoDisplay* d, Term t);

#endif // !__UNO_DISP__
