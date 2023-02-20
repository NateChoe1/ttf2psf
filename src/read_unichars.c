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

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <read_unichars.h>

#define MAX_UNICHARS 128

static int char_val(char c);
/* Gets a hexadecimal digit's value or -1 if it's not a valid hex digit */

uint32_t *read_unichars(FILE *input, int *len_return) {
#define FGETC(var, file) \
	{ if (((var) = fgetc(file)) == EOF) goto eof; }
	static uint32_t ret[MAX_UNICHARS];
	int len;

	len = 0;

	for (;;) {
		int c;
		FGETC(c, input);
		if (isspace(c)) {
			continue;
		}
		if (c == '#') {
			while (c != '\n') {
				FGETC(c, input);
				continue;
			}
		}
		ungetc(c, input);
		break;
	}
	/* Skip past empty lines */

	for (;; ++len) {
		int c;
		uint32_t value;
		for (;;) {
			FGETC(c, input);
			if (c == '\n') {
				goto done;
			}
			if (c == '#') {
				while (c != '\n') {
					FGETC(c, input);
				}
				goto done;
			}
			if (isspace(c)) {
				continue;
			}
			ungetc(c, input);
			break;
		}
		/* Swallow until the first non-space character of the line (or
		 * end the function if there aren't any left) */

		if (len >= MAX_UNICHARS) {
			goto error;
		}
		/* Line's too long */

		c = fgetc(input);
		if (tolower(c) != 'u') {
			goto error;
		}
		c = fgetc(input);
		if (tolower(c) != '+') {
			goto error;
		}
		value = 0;
		for (int i = 0; i < 5; ++i) {
			int c_val;
			c = fgetc(input);
			if (c == EOF) {
				goto error;
			}
			if (isspace(c)) {
				ungetc(c, input);
				break;
			}
			c_val = char_val(c);
			if (c_val < 0) {
				goto error;
			}
			value <<= 4;
			value |= c_val;
		}
		ret[len] = value;
	}

done:
	*len_return = len;
	return ret;
eof:
	if (len == 0) {
		*len_return = 0;
		return NULL;
	}
	else {
		goto done;
	}
error:
	*len_return = -1;
	return NULL;
#undef FGETC
}
/* TODO: I'm only like half way through writing this function but I know for a
 * fact that it should definitely be rewritten by the time I'm done. */

static int char_val(char c) {
	const char *check = "0123456789abcdef";
	char *loc = strchr(check, tolower(c));
	if (loc == NULL) {
		return -1;
	}
	return loc - check;
}
