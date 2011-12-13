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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <ft2build.h>
#include FT_FREETYPE_H

#include "core/GP_Debug.h"
#include "GP_FreeType.h"

GP_Font *GP_FontFreeTypeLoad(const char *path, uint32_t width, uint32_t height)
{
	int err;

	FT_Library library;
	FT_Face face;

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

	GP_Font *font = malloc(sizeof(GP_Font));

	if (font == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err1;
	}

	strncpy(font->family, face->family_name, sizeof(font->family));
	font->family[GP_FONT_NAME_MAX] = '\0';
	strncpy(font->name, face->style_name, sizeof(font->name));
	font->name[GP_FONT_NAME_MAX] = '\0';
	strcpy(font->author, "Unknown");
	strcpy(font->license, "Unknown");

	font->charset = GP_CHARSET_7BIT;
	font->version = 0;

	unsigned int i;
	
	font->height = 0;
	font->bytes_per_line = 0;
	font->max_bounding_width = 0;
	font->baseline = 0;

	int32_t baseline = 0;

	for (i = 0x21; i < 0x7f; i++) {
		FT_UInt glyph_idx = FT_Get_Char_Index(face, i);
	
		err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);

		if (err) {
			GP_DEBUG(1, "Failed to load glyph '%c'", i);
			goto err2;
		}
		
		err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);

		if (err) {
			GP_DEBUG(1, "Failed to render glyph '%c'", i);
			goto err2;
		}

		FT_Bitmap *bitmap = &face->glyph->bitmap;

		GP_DEBUG(2, "Glyph '%c' bitmap rows=%i width=%i pitch=%i",
		         i, bitmap->rows, bitmap->width, bitmap->pitch);

		GP_DEBUG(2, " bitmap top=%i left=%i",
			 face->glyph->bitmap_top, face->glyph->bitmap_left);

		width = (face->glyph->metrics.width>>6);

		if (font->max_bounding_width < width)
			font->max_bounding_width = width;

		if (font->bytes_per_line < bitmap->pitch)
			font->bytes_per_line = bitmap->pitch;

		height = face->glyph->metrics.height>>6;

		if (font->height < height)
			font->height = height;
	
		baseline = bitmap->rows - face->glyph->bitmap_top; 

		if (font->baseline < baseline)
			font->baseline = baseline;
	}

	size_t font_data_size = GP_GetFontDataSize(font);
	font->data = malloc(font_data_size);

	if (font->data == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err2;
	}

	memset(font->data, 0, font_data_size);
	
	for (i = 0x21; i < 0x7f; i++) {
		FT_UInt glyph_idx = FT_Get_Char_Index(face, i);
	
		err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);

		if (err) {
			GP_DEBUG(1, "Failed to load glyph '%c'", i);
			goto err2;
		}
		
		err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);

		if (err) {
			GP_DEBUG(1, "Failed to render glyph '%c'", i);
			goto err2;
		}

		FT_Bitmap *bitmap = &face->glyph->bitmap;

		GP_CharData *char_data = (GP_CharData*)GP_GetCharData(font, i);
	
		char_data->pre_offset  = face->glyph->bitmap_left;
		char_data->post_offset = face->glyph->advance.x>>6;
		
		char_data->char_width = bitmap->width;
	
		int x, y;
	
		for (y = 0; y < bitmap->rows; y++) {
			for (x = 0; x < bitmap->pitch; x++) {
				uint8_t trans_y = y + font->height - font->baseline - face->glyph->bitmap_top;
				uint8_t addr = font->bytes_per_line * trans_y + x;

				char_data->bitmap[addr] = bitmap->buffer[y*bitmap->pitch + x];
			}
		}
	}

	return font;
err2:
	free(font);
err1:
	//TODO FREETYPE CLEANUP
	return NULL;
}
