// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef NO_CONFIG
#include "../../config.h"
#endif
#include <core/gp_debug.h>
#include <text/gp_font.h>

#ifdef HAVE_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H

gp_font_face *gp_font_face_load(const char *path, uint32_t width, uint32_t height)
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
		GP_DEBUG(1, "Failed to open font '%s'", path);
		goto err1;
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
		goto err2;
	}

	/* Allocate font face structure */
	unsigned int font_face_size;

	font_face_size = sizeof(gp_font_face) +
	                 sizeof(uint32_t) * gp_get_glyph_count(GP_CHARSET_7BIT);

	gp_font_face *font = malloc(font_face_size);

	if (!font) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err2;
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
			goto err3;
		}

		err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		if (err) {
			GP_DEBUG(1, "Failed to render glyph '%c'", i);
			goto err3;
		}

		FT_Bitmap *bitmap = &face->glyph->bitmap;

		GP_DEBUG(2, "Glyph '%c' bitmap rows=%i width=%i pitch=%i",
		         i, bitmap->rows, bitmap->width, bitmap->pitch);

		GP_DEBUG(2, " bitmap top=%i left=%i",
			 face->glyph->bitmap_top, face->glyph->bitmap_left);

		/* count glyph table size and fill offset table */
		font->glyph_offsets[i - 0x20] = glyph_table_size;
		glyph_table_size += sizeof(gp_glyph) +
		                    bitmap->rows * bitmap->pitch;
	}

	GP_DEBUG(2, "Glyph table size %u bytes", glyph_table_size);

	font->glyphs = malloc(glyph_table_size);

	if (!font->glyphs) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err3;
	}

	font->max_glyph_width = 0;
	font->max_glyph_advance = 0;
	font->ascend  = 0;
	font->descend = 0;

	uint64_t avg_advance = 0;

	for (i = 0x20; i < 0x7f; i++) {
		FT_UInt glyph_idx = FT_Get_Char_Index(face, i);

		err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);

		GP_DEBUG(2, "Loading and rendering glyph '%c'", i);

		if (err) {
			GP_DEBUG(1, "Failed to load glyph '%c'", i);
			goto err4;
		}

		err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		if (err) {
			GP_DEBUG(1, "Failed to render glyph '%c'", i);
			goto err4;
		}

		gp_glyph *glyph_bitmap = gp_get_glyph(font, i);
		FT_GlyphSlot glyph = face->glyph;

		glyph_bitmap->width     = glyph->bitmap.width;
		glyph_bitmap->height    = glyph->bitmap.rows;
		glyph_bitmap->bearing_x = glyph->bitmap_left;
		glyph_bitmap->bearing_y = glyph->bitmap_top;
		glyph_bitmap->advance_x = (glyph->advance.x + 32)>>6;

		avg_advance += glyph->advance.x;

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

	avg_advance = (((avg_advance + 32)>>6) + 47) / 95;
	font->avg_glyph_advance = (avg_advance + (avg_advance+5)/10);

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	return font;
err4:
	free(font->glyphs);
err3:
	free(font);
err2:
	FT_Done_Face(face);
err1:
	FT_Done_FreeType(library);
	return NULL;
}

#else

gp_font_face *gp_font_face_load(const char *path, uint32_t width, uint32_t height)
{
	(void)path;
	(void)width;
	(void)height;

	GP_WARN("FreeType support not compiled in.");

	return NULL;
}

#endif /* HAVE_FREETYPE */

#ifdef HAVE_FONTCONFIG

#include <fontconfig/fontconfig.h>

gp_font_face *gp_font_face_fc_load(const char *family_name, uint32_t width, uint32_t height)
{
	gp_font_face *ret = NULL;
	FcResult res;
	FcChar8 *font_path;

	FcPattern *pat = FcNameParse((const FcChar8*)family_name);
	if (!pat)
		goto exit1;

	FcConfigSubstitute(NULL, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);

	FcPattern *font = FcFontMatch(NULL, pat, &res);
	if (!font)
		goto exit2;

	if (FcPatternGetString(font, FC_FILE, 0, &font_path) != FcResultMatch)
		goto exit3;

	GP_DEBUG(1, "Font '%s' path '%s'", family_name, (char*)font_path);

	ret = gp_font_face_load((const char *)font_path, width, height);

exit3:
	FcPatternDestroy(font);
exit2:
	FcPatternDestroy(pat);
exit1:
	return ret;
}

#else

gp_font_face *gp_font_face_fc_load(const char *family_name, uint32_t width, uint32_t height)
{
	(void)family_name;
	(void)width;
	(void)height;

	GP_WARN("Fontconfig support not compiled in.");

	return NULL;
}

#endif /* HAVE_FONTCONFIG */
