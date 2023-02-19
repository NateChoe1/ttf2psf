#ifndef HAVE_GLYPHS
#define HAVE_GLYPHS

#include <ft2build.h>
#include FT_FREETYPE_H

struct code {
	FT_ULong charcode;
	struct code *next;
}; /* All the 32 bit character codes that correspond to a glyph */

struct glyph {
	FT_UInt index;
	struct code *code;
	struct code **last_code;
	struct glyph *next;
}; /* Linked list containing all the glyphs in a font face */

struct charset {
	FT_Face face;
	struct glyph *first;
	struct glyph **last;
	/* last = &[the last node]->next */
	int size;
};

struct charset *new_charset(FT_Face face);
void free_charset(struct charset *charset);
int add_char(struct charset *charset, FT_ULong charcode);
int add_equivalent(struct glyph *glyph, FT_ULong charcode);
struct glyph *search_glyph(struct charset *charset, FT_ULong charcode);
/* These functions return either non-zero or NULL on error. */

#endif
