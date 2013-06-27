/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include "../../config.h"
#include "core/GP_Debug.h"
#include "text/GP_Font.h"

#ifdef HAVE_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H

GP_FontFace *GP_FontFaceLoad(const char *path, uint32_t width, uint32_t height)
{
	FT_Library library;
	FT_Face face;
	int err;

	err = FT_Init_FreeType(&library);

	if (err) {
		GP_DEBUG(1, "Failed to initalize Free Type");
		return NULL;
	}

	err = FT_New_Face(library, path, 0, &face);

	if (err) {
		//TODO: FT_Exit_FreeType() ?
		GP_DEBUG(1, "Failed to open font '%s'", path);
		return NULL;
	}

	GP_DEBUG(1, "Opened font '%s'", path);
	GP_DEBUG(2, "Font family_name='%s' style_name='%s' num_glyphs=%li",
	            face->family_name, face->style_name,
		    (long)face->num_glyphs);
	GP_DEBUG(2, "Font ascender=%i descender=%i height=%i",
	            (int)face->ascender, (int)face->descender, (int)face->height);

	//TODO: not scalable fonts?
	err = FT_Set_Pixel_Sizes(face, width, height);

	if (err) {
		GP_DEBUG(1, "Failed to set pixel size");
		return NULL;
	}

	/* Allocate font face structure */
	unsigned int font_face_size;

	font_face_size = sizeof(GP_FontFace) +
	                 sizeof(uint32_t) * GP_GetGlyphCount(GP_CHARSET_7BIT);

	GP_FontFace *font = malloc(font_face_size);

	if (font == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err1;
	}

	/* Copy font metadata */
	strncpy(font->family_name, face->family_name,
	        sizeof(font->family_name));
	font->family_name[GP_FONT_NAME_MAX - 1] = '\0';
	strncpy(font->style_name, face->style_name,
	        sizeof(font->style_name));
	font->style_name[GP_FONT_NAME_MAX - 1] = '\0';

	font->glyph_bitmap_format = GP_FONT_BITMAP_8BPP;
	font->charset = GP_CHARSET_7BIT;

	/* Count glyph data size */
	unsigned int i;
	unsigned int glyph_table_size = 0;

	for (i = 0x20; i < 0x7f; i++) {
		FT_UInt glyph_idx = FT_Get_Char_Index(face, i);

		err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);

		if (err) {
			GP_DEBUG(1, "Failed to load glyph '%c'", i);
			goto err2;
		}

		err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		if (err) {
			GP_DEBUG(1, "Failed to render glyph '%c'", i);
			goto err2;
		}

		FT_Bitmap *bitmap = &face->glyph->bitmap;

		GP_DEBUG(2, "Glyph '%c' bitmap rows=%i width=%i pitch=%i",
		         i, bitmap->rows, bitmap->width, bitmap->pitch);

		GP_DEBUG(2, " bitmap top=%i left=%i",
			 face->glyph->bitmap_top, face->glyph->bitmap_left);

		/* count glyph table size and fill offset table */
		font->glyph_offsets[i - 0x20] = glyph_table_size;
		glyph_table_size += sizeof(GP_GlyphBitmap) +
		                    bitmap->rows * bitmap->pitch;
	}

	GP_DEBUG(2, "Glyph table size %u bytes", glyph_table_size);

	font->glyphs = malloc(glyph_table_size);

	if (font->glyphs == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err2;
	}

	font->max_glyph_width = 0;
	font->max_glyph_advance = 0;
	font->ascend  = 0;
	font->descend = 0;

	for (i = 0x20; i < 0x7f; i++) {
		FT_UInt glyph_idx = FT_Get_Char_Index(face, i);

		err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);

		GP_DEBUG(2, "Loading and rendering glyph '%c'", i);

		if (err) {
			GP_DEBUG(1, "Failed to load glyph '%c'", i);
			goto err3;
		}

		err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		if (err) {
			GP_DEBUG(1, "Failed to render glyph '%c'", i);
			goto err3;
		}

		GP_GlyphBitmap *glyph_bitmap = GP_GetGlyphBitmap(font, i);
		FT_GlyphSlot glyph = face->glyph;

		glyph_bitmap->width     = glyph->bitmap.width;
		glyph_bitmap->height    = glyph->bitmap.rows;
		glyph_bitmap->bearing_x = glyph->bitmap_left;
		glyph_bitmap->bearing_y = glyph->bitmap_top;
		glyph_bitmap->advance_x = (glyph->advance.x + 32)>>6;

		int16_t width = glyph_bitmap->bearing_x + glyph_bitmap->width;
		int16_t ascend = glyph_bitmap->bearing_y;
		int16_t descend = glyph_bitmap->height - ascend;

		if (font->ascend < ascend)
			font->ascend = ascend;

		if (font->descend < descend)
			font->descend = descend;

		if (font->max_glyph_advance < glyph_bitmap->advance_x)
			font->max_glyph_advance = glyph_bitmap->advance_x;

		if (font->max_glyph_width < width)
			font->max_glyph_width = width;

		int x, y;

		for (y = 0; y < glyph_bitmap->height; y++) {
			for (x = 0; x < glyph_bitmap->width; x++) {
				unsigned int addr = glyph_bitmap->width * y + x;

				glyph_bitmap->bitmap[addr] = glyph->bitmap.buffer[y * glyph->bitmap.pitch + x];
			}
		}
	}

	return font;
err3:
	free(font->glyphs);
err2:
	free(font);
err1:
	//TODO FREETYPE CLEANUP
	return NULL;
}

#else

GP_FontFace *GP_FontFaceLoad(const char *path, uint32_t width, uint32_t height)
{
	(void)path;
	(void)width;
	(void)height;

	GP_WARN("FreeType support not compiled in.");

	return NULL;
}

#endif /* HAVE_FREETYPE */
