// ttf2psf - A simple, working utility to generate Linux console fonts
// Copyright (C) 2023  Nate Choe
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILIY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>

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
