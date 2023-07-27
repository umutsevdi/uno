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
#define UNO_RF_EQ(VAL, FLAG) ((VAL & FLAG) == FLAG)
/**
 * UnoRequest flags
 */
typedef enum {
    UNO_RF_NEWLINE = 0x20000000, //     0010 0000 \r\n
    UNO_RF_BACKSPACE = 0x30000000, //   0011 0000 BACKSPACE
    UNO_RF_ESCAPE = 0x40000000, //      0100 0000 ESC
    UNO_RF_DIR_ENABLED = 0x80000000, // 0101 0000
    UNO_RF_CMD_ENABLED = 0x80000000, // 0111 0000
    UNO_RF_DIR_LEFT = 0x01000000, //    0000 0001
    UNO_RF_DIR_DOWN = 0x02000000, //    0000 0010
    UNO_RF_DIR_UP = 0x04000000, //      0000 0100
    UNO_RF_DIR_RIGHT = 0x08000000, //   0000 1000

    UNO_RF_DIR_MASK = 0x0F000000, //    0000 1111
} UNO_RF;

typedef enum {
    UNO_RF_WCHAR_L = L'D',
    UNO_RF_WCHAR_D = L'B',
    UNO_RF_WCHAR_U = L'A',
    UNO_RF_WCHAR_R = L'C',
} UNO_RF_DIR;

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
void uno_move(UnoBuffer* b, UnoRequest r);

#endif // !__UNO_MOVEMENT__
