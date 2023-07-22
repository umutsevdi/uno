/******************************************************************************

 * File: include/uno_movement.h
 *
 * Author: Umut Sevdi
 * Created: 06/28/23
 * Description: Keyboard movement utilities

*****************************************************************************/

/*
 *   CASES:
 * move with arrow keys
 * press enter to new line
 * press backspace to delete last character and move back
 * ESC to run command
 * Arbitrary char press
 *
 *
 *
 *
 *
 */

#ifndef __UNO_MOVEMENT__
#define __UNO_MOVEMENT__
#include "uno_buffer.h"
#include "uno_display.h"
#define NL_CHAR 0x2000000
#define BS_CHAR 0x3000000
#define D_ENABLED 0x2000000
#define M_ENABLED 0x4000000
#define E_CHAR 0x8000000

// counts in reverse order
#define UNO_NTH_HEX(_VALUE, _N) (_VALUE & (0xFU << (_N * 4))) >> (_N * 4)
#define UNO_TIMES(a, _TIMES)               \
    for (size_t _I = 0; _I < _TIMES; _I++) \
        a;

#define UNO_DIR_MASK 0x0F00000
typedef enum {
    UNO_POS_BUFFER = 0x0010000,
    UNO_POS_POPUP = 0x0020000,
    UNO_POS_DOWN = 0x0200000,
    UNO_POS_UP = 0x0400000,
    UNO_POS_RIGHT = 0x0800000,
} UNO_POS;
#define UNO_DIR_MASK 0x0F00000
typedef enum {
    UNO_DIR_LEFT = 0x0100000,
    UNO_DIR_DOWN = 0x0200000,
    UNO_DIR_UP = 0x0400000,
    UNO_DIR_RIGHT = 0x0800000,
} UNO_DIR;

/**
 * A 32 bit unsigned integer that represents the request type for movement
 *
 * * First hex configures the request.
 *    - E_CHAR: Whether escape character has been pressed or not
 *    - M_ENABLED: Whether [ is pressed, as well. It enables UNO_DIRs
 *    - D_ENABLED Whether backspace or enter is pressed
 *    - NL_CHAR If D_ENABLED it means New line char is pressed
 *    - BS_CHAR If D_ENABLED it means backspace is pressed
 *  * Second hex decides the movement. It is non-zero only when E_CHAR and M_ENABLED are 1.
 *    - 0 0 0 0
 *    - ^-------UNO_DIR_LEFT
 *    - --^-----UNO_DIR_DOWN
 *    - ----^---UNO_DIR_UP
 *    - ------^-UNO_DIR_RIGHT
 *  * Third hex describes where the movement will occur. This feature will
 *  be added later
 *    -
 *  // TODO Fix documentation. Things to add:
 *  - Command mode. Similar to Vim
 *  - Keybindings
 */
typedef uint32_t UnoRequest;
/**
 * Builds a request from given wchar_t and parameters
 */
UnoRequest uno_make_req(wchar_t c, int is_cntrl, int is_escape, int is_alt);
/**
 * Moves the cursor horizontally or vertically
 * based on given request by one.
 * @param b - Buffer to move cursor on
 * @param r - Movement request, see UnoRequest
 *
 */
void uno_move_cursor(UnoBuffer* b, UnoRequest r);

#endif // !__UNO_MOVEMENT__
