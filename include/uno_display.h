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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum DSIG {
    UNO_SIG_EXIT = -1,
    UNO_SIG_READY = 0,
    UNO_SIG_BUFFER = 1
} UNO_SIG;

typedef struct {
    int row;
    int col;
} Term;

typedef struct _UNO_DISP {
    char c;
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
