#ifndef HAVE_READ_UNICHARS
#define HAVE_READ_UNICHARS

#include <stdio.h>
#include <stdint.h>

uint32_t *read_unichars(FILE *input, int *len_return);
/* Reads a line of unicode characters from input. Returns NULL at EOF or on
 * error, sets len_return to 0 on EOF, -1 on error. Output is returned in a
 * static buffer which will be overwritten on future calls to read_unichars, and
 * which should not be freed with free() */

#endif
