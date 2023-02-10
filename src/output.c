#include <ft2build.h>
#include FT_FREETYPE_H

#include <output.h>

int write_output(FT_Library library, FT_Face face, FILE *output) {
	FT_ULong code; /* Character code */
	FT_UInt index; /* Glyph index */
	FT_Error error;

	code = FT_Get_First_Char(face, &index);
	for (code = FT_Get_First_Char(face, &index); index != 0;
			code = FT_Get_Next_Char(face, code, &index)) {
		error = FT_Load_Char(face, code, FT_LOAD_DEFAULT);
		if (error) {
			continue;
		}

		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
		if (error) {
			continue;
		}

		if (face->glyph->bitmap.width != 8 ||
				face->glyph->bitmap.rows != 16) {
			continue;
		}

		int curr_row_offset;
		if (face->glyph->bitmap.pitch >= 0) {
			curr_row_offset = 0;
		}
		else {
			curr_row_offset = (face->glyph->bitmap.rows-1) *
					-face->glyph->bitmap.pitch;
		}

		for (int i = 0; i < face->glyph->bitmap.rows; ++i) {
			unsigned char *row = face->glyph->bitmap.buffer +
				curr_row_offset;

			fputc(row[0], output);

			curr_row_offset += face->glyph->bitmap.pitch;
		}
	}
	return 0;
}

