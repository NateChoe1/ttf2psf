#include <stdio.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <output.h>

struct code {
	FT_ULong charcode;
	struct code *next;
}; /* All the 32 bit character codes that correspond to a glyph */

struct glyph {
	FT_UInt index;
	struct code *code;
	struct glyph *next;
}; /* Linked list containing all the glyphs in a font face */

struct glyph *read_face(FT_Face face, int *glyph_count);
static int add_code(struct glyph *glyph, FT_ULong code);
static int add_glyph(struct glyph **face, FT_UInt index);
static int add_new_char(struct glyph **face, FT_UInt index, FT_ULong charcode);
static void free_glyph(struct glyph *glyph);
static int write_glyph(FT_Face face, int width, int height, FILE *output);
static inline int get_bit(FT_Face face,
		int row, int col, int width, int height);
static int div_up(int num, int denom);

int write_psf(int width, int height, struct psf_interface *interface,
		FT_Face face, FILE *output) {
	int glyph_count;
	struct glyph *glyphs = read_face(face, &glyph_count);
	FT_Error error;
	if (glyphs == NULL) {
		return 1;
	}

	if (interface->write_header(width, height, 1, glyph_count, output)) {
		return 1;
	}

	for (struct glyph *iter = glyphs; iter != NULL; iter = iter->next) {
		error = FT_Load_Glyph(face, iter->index,
				FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
		if (error) {
			goto error;
		}
		if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP ||
			face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_MONO) {
			goto error;
		}
		if (write_glyph(face, width, height, output)) {
			goto error;
		}
	}

	for (struct glyph *g = glyphs; g != NULL; g = g->next) {
		struct code *code = g->code;
		while (code != NULL) {
			interface->write_unichar(code->charcode, output);
			code = code->next;
		}
		interface->write_separator(output);
	}

	free_glyph(glyphs);
	return 0;

error:
	free_glyph(glyphs);
	return 1;
}

struct glyph *read_face(FT_Face face, int *glyph_count) {
	struct glyph *ret = NULL;
	struct glyph **next = &ret;
	FT_ULong charcode;
	FT_UInt gindex;

	*glyph_count = 0;

	for (int i = 0; i <= 0xff; ++i) {
		gindex = FT_Get_Char_Index(face, i);
		add_new_char(next, gindex, i);
		next = &(*next)->next;
		++*glyph_count;
	}

	charcode = FT_Get_First_Char(face, &gindex);
	while (gindex != 0 && *glyph_count < 512) {
		if (charcode <= 0xff) {
			goto next_char;
		}

		struct glyph *iter = ret;

		while (iter != NULL) {
			if (gindex == iter->index) {
				if (add_code(iter, charcode)) {
					goto error;
				}
				goto next_char;
			}
			iter = iter->next;
		}

		++*glyph_count;
		add_new_char(next, gindex, charcode);
		next = &(*next)->next;

next_char:
		charcode = FT_Get_Next_Char(face, charcode, &gindex);
	}

	return ret;
error:
	free_glyph(ret);
	return NULL;
}

static int add_code(struct glyph *glyph, FT_ULong code) {
	struct code *new_code;
	new_code = malloc(sizeof *new_code);
	if (new_code == NULL) {
		return 1;
	}
	new_code->charcode = code;
	new_code->next = glyph->code;
	glyph->code = new_code;
	return 0;
}

static int add_glyph(struct glyph **face, FT_UInt index) {
	struct glyph *new_glyph;
	new_glyph = malloc(sizeof *new_glyph);
	if (new_glyph == NULL) {
		return 1;
	}
	new_glyph->index = index;
	new_glyph->code = NULL;
	new_glyph->next = NULL;
	*face = new_glyph;
	return 0;
}

static int add_new_char(struct glyph **face, FT_UInt index, FT_ULong charcode) {
	if (add_glyph(face, index)) {
		return 1;
	}
	if (add_code(*face, charcode)) {
		return 1;
	}
	return 0;
}

static void free_glyph(struct glyph *glyph) {
	while (glyph != NULL) {
		struct code *iter = glyph->code;
		while (iter != NULL) {
			struct code *next_iter = iter->next;
			free(iter);
			iter = next_iter;
		}

		struct glyph *next_glyph;
		next_glyph = glyph->next;
		free(glyph);
		glyph = next_glyph;
	}
}

static int write_glyph(FT_Face face, int width, int height, FILE *output) {
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < (width+7) / 8; ++j) {
			unsigned char curr_char;
			curr_char = 0x00;
			for (int k = 0; k < 8; ++k) {
				curr_char <<= 1;
				curr_char |= get_bit(face,
						i, j*8 + k, width, height);
			}
			if (fputc(curr_char, output) == EOF) {
				return 1;
			}
		}
	}
	return 0;
}

static inline int get_bit(FT_Face face,
		int row, int col, int width, int height) {
	const FT_GlyphSlot glyph = face->glyph;
	const FT_Bitmap *bmp = &glyph->bitmap;
	const int
		left_edge = glyph->bitmap_left,
		top_edge = (height-1) - glyph->bitmap_top -
			div_up(face->bbox.yMin, face->units_per_EM),
		right_edge = left_edge + bmp->pitch * 8,
		bot_edge = top_edge + bmp->rows;

	if (col < left_edge || col >= right_edge ||
			row < top_edge || row >= bot_edge) {
		return 0;
	}

	const int bmp_x = col - left_edge, bmp_y = row - top_edge;
	unsigned char *row_data;

	if (bmp->pitch < 0) {
		row_data = bmp->buffer + -bmp->pitch * (bmp->rows-1);
	}
	else {
		row_data = bmp->buffer;
	}
	row_data += bmp->pitch * bmp_y;

	return (row_data[bmp_x/8] >> ((8-bmp_x) % 8)) & 1;
}

static int div_up(int num, int denom) {
	int ret = num / denom;
	return ret + (num % denom) ? 1:0;
}
