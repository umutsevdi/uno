/******************************************************************************

 * File: include/uno_io.h
 *
 * Author: Umut Sevdi
 * Created: 07/30/23
 * Description: IO operations

*****************************************************************************/

#ifndef __UNO_IO__
#define __UNO_IO__

#include "uno_buffer.h"

/**
 * Opens a file in given path in a separate buffer and returns as a UnoBuffer
 * @param file   - path to file
 * @param file_l - length of the string
 * @return buffer, NULL if any error occurs
 */
UnoBuffer* uno_io_open(char* file, size_t file_l);

/**
 * Updates the given file incase it was edited by an external source
 */
void uno_io_fetch(UnoBuffer* b);

/**
 * Saves the given buffer.
 */
int uno_io_save(UnoBuffer* buffer);

/**
 * Renames or moves the file
 */
int uno_io_rename(UnoBuffer* buffer, char* name, size_t name_t);

/**
 * Saves the given buffer.
 */
int uno_io_delete(UnoBuffer* buffer);
#endif
