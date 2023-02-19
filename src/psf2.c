#include <stdint.h>

/* TODO: Actually implement psf2 */
struct psf2_header {
	char magic[4];		/* Always \x72\xb5\x4a\x86 */
	uint32_t version;	/* Always 0 */
	uint32_t header_size;	/* Always sizeof(psf2_header) */
	uint32_t flags;		/* Set to 1 if there's a unicode table */
	uint32_t length;	/* Number of glyphs */
	uint32_t char_size; 	/* (width+7) / 8 * height */
	uint32_t height;	/* Width and height in pixels */
	uint32_t width;
};
