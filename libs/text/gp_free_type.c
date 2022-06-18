// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2022 Cyril Hrubis <metan@ucw.cz>
 */

#include "../../config.h"
#include <core/gp_debug.h>
#include <text/gp_font.h>

#ifdef HAVE_FREETYPE

#include <ft2build.h>
#include FT_FREETYPE_H

#define GLYPH_CACHE_SIZE 10

struct glyph_cache_slot {
	uint32_t idx;
	uint32_t hits;
	gp_glyph *glyph;
};

struct font_freetype_priv {
	FT_Library library;
	FT_Face face;
	struct glyph_cache_slot glyph_cache[GLYPH_CACHE_SIZE];
};

static void glyph_cache_free(struct font_freetype_priv *priv)
{
	size_t i;

	for (i = 0; i < GLYPH_CACHE_SIZE; i++)
		free(priv->glyph_cache[i].glyph);
}

static void font_freetype_free(gp_font_face *self)
{
	struct font_freetype_priv *priv = self->priv;
	size_t i;

	FT_Done_Face(priv->face);
	FT_Done_FreeType(priv->library);

	for (i = 0; i < self->glyph_tables; i++) {
		free(self->glyphs[i].offsets);
		free(self->glyphs[i].glyphs);
	}

	glyph_cache_free(self->priv);

	free(self->priv);
	free(self);
}

static void copy_glyph(FT_Face face, gp_glyph *glyph)
{
	glyph->width = face->glyph->bitmap.width;
	glyph->height = face->glyph->bitmap.rows;
	glyph->bearing_x = face->glyph->bitmap_left;
	glyph->bearing_y = face->glyph->bitmap_top;
	glyph->advance_x = (face->glyph->advance.x + 32)>>6;

	int x, y;

	for (y = 0; y < glyph->height; y++) {
		for (x = 0; x < glyph->width; x++) {
			unsigned int addr = glyph->width * y + x;

			glyph->bitmap[addr] = face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
		}
	}
}

static gp_glyph *create_glyph_bitmap(FT_Face face)
{
	FT_Bitmap *bitmap = &face->glyph->bitmap;
	gp_glyph *glyph;

	glyph = malloc(sizeof(gp_glyph) + bitmap->rows * bitmap->pitch);
	if (!glyph) {
		GP_DEBUG(1, "Malloc failed :-(");
		return NULL;
	}

	copy_glyph(face, glyph);

	return glyph;
}

static int load_and_render_glyph(FT_Face face, uint32_t ch)
{
	FT_UInt glyph_idx = FT_Get_Char_Index(face, ch);
	int err;

	err = FT_Load_Glyph(face, glyph_idx, FT_LOAD_DEFAULT);
	if (err) {
		GP_DEBUG(1, "Failed to load glyph '%c'", ch);
		return err;
	}

	err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	if (err) {
		GP_DEBUG(1, "Failed to render glyph '%c'", ch);
		return err;
	}

	return 0;
}

static gp_glyph *glyph_cache_lookup(struct font_freetype_priv *priv, uint32_t ch)
{
	size_t i;

	for (i = 0; i < GLYPH_CACHE_SIZE; i++) {
		if (ch == priv->glyph_cache[i].idx) {
			priv->glyph_cache[i].hits++;
			return priv->glyph_cache[i].glyph;
		}
	}

	return NULL;
}

static void glyph_cache_free_slot(struct font_freetype_priv *priv, size_t i)
{
	GP_DEBUG(4, "Freeing glyph cache slot %zi", i);

	free(priv->glyph_cache[i].glyph);
	priv->glyph_cache[i].hits = 0;
	priv->glyph_cache[i].idx = 0;
}

static size_t glyph_cache_get_slot(struct font_freetype_priv *priv)
{
	uint32_t min_hits = priv->glyph_cache[0].hits;
	size_t i, min_idx = 0;

	for (i = 0; i < GLYPH_CACHE_SIZE; i++) {
		if (!priv->glyph_cache[i].idx)
			return i;

		if (min_hits > priv->glyph_cache[i].hits) {
			min_hits = priv->glyph_cache[i].hits;
			min_idx = i;
		}
	}

	glyph_cache_free_slot(priv, min_idx);

	return min_idx;
}

static void glyph_cache_insert(struct font_freetype_priv *priv, gp_glyph *glyph, uint32_t ch)
{
	size_t i = glyph_cache_get_slot(priv);

	GP_DEBUG(4, "Inserting glyph 0x%08x into cache at %zi", ch, i);

	priv->glyph_cache[i].glyph = glyph;
	priv->glyph_cache[i].idx = ch;
}

static gp_glyph *glyph_freetype_load(const gp_font_face *self, uint32_t ch)
{
	struct font_freetype_priv *priv = self->priv;
	gp_glyph *glyph;

	GP_DEBUG(4, "Loading glyph 0x%08x", ch);

	glyph = glyph_cache_lookup(priv, ch);
	if (glyph) {
		GP_DEBUG(4, "Glyph was cached");
		return glyph;
	}

	if (load_and_render_glyph(priv->face, ch))
		return NULL;

	glyph = create_glyph_bitmap(priv->face);
	if (!glyph)
		return NULL;

	glyph_cache_insert(priv, glyph, ch);

	return glyph;
}

static gp_font_face_ops font_freetype_ops = {
	.font_free = font_freetype_free,
	.glyph_load = glyph_freetype_load,
};

gp_font_face *gp_font_face_load(const char *path, uint32_t width, uint32_t height)
{
	int err;
	struct font_freetype_priv *priv;

	priv = malloc(sizeof(struct font_freetype_priv));
	if (!priv) {
		GP_DEBUG(1, "Malloc failed :-(");
		return NULL;
	}

	memset(priv, 0, sizeof(*priv));

	err = FT_Init_FreeType(&priv->library);
	if (err) {
		GP_DEBUG(1, "Failed to initalize Free Type");
		goto err0;
	}

	err = FT_New_Face(priv->library, path, 0, &priv->face);
	if (err) {
		GP_DEBUG(1, "Failed to open font '%s'", path);
		goto err1;
	}

	GP_DEBUG(1, "Opened font '%s'", path);
	GP_DEBUG(2, "Font family_name='%s' style_name='%s' num_glyphs=%li",
	            priv->face->family_name, priv->face->style_name,
		    (long)priv->face->num_glyphs);
	GP_DEBUG(2, "Font ascender=%i descender=%i height=%i",
	            (int)priv->face->ascender, (int)priv->face->descender,
	            (int)priv->face->height);

	//TODO: not scalable fonts?
	err = FT_Set_Pixel_Sizes(priv->face, width, height);
	if (err) {
		GP_DEBUG(1, "Failed to set pixel size");
		goto err2;
	}

	gp_font_face *font = malloc(sizeof(gp_font_face) + sizeof(gp_glyphs));
	if (!font) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err2;
	}

	font->priv = priv;
	font->ops = &font_freetype_ops;

	font->glyphs[0].offsets = malloc(sizeof(gp_glyph_offset) * (0x7f-0x20));
	if (!font->glyphs[0].offsets) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err3;
	}

	/* Copy font metadata */
	strncpy(font->family_name, priv->face->family_name,
	        sizeof(font->family_name));
	font->family_name[GP_FONT_NAME_MAX - 1] = '\0';

	font->glyph_tables = 1;

	if (FT_IS_FIXED_WIDTH(priv->face))
		font->style = GP_FONT_MONO;
	else
		font->style = GP_FONT_REGULAR;

	if (priv->face->style_flags & FT_STYLE_FLAG_BOLD)
		font->style |= GP_FONT_BOLD;

	if (priv->face->style_flags & FT_STYLE_FLAG_ITALIC)
		font->style |= GP_FONT_ITALIC;

	font->glyph_bitmap_format = GP_FONT_BITMAP_8BPP;

	/* Count glyph data size */
	unsigned int i;
	unsigned int glyph_table_size = 0;

	for (i = 0x20; i < 0x7f; i++) {
		if (load_and_render_glyph(priv->face, i))
			goto err3;

		FT_Bitmap *bitmap = &priv->face->glyph->bitmap;

		GP_DEBUG(4, "Glyph '%c' bitmap rows=%i width=%i pitch=%i",
		         i, bitmap->rows, bitmap->width, bitmap->pitch);

		GP_DEBUG(4, " bitmap top=%i left=%i",
			 priv->face->glyph->bitmap_top,
		         priv->face->glyph->bitmap_left);

		/* count glyph table size and fill offset table */
		font->glyphs[0].offsets[i - 0x20] = glyph_table_size;
		glyph_table_size += sizeof(gp_glyph) +
		                    bitmap->rows * bitmap->pitch;
	}

	GP_DEBUG(2, "Glyph table size %u bytes", glyph_table_size);

	font->glyphs[0].glyphs = malloc(glyph_table_size);
	if (!font->glyphs[0].glyphs) {
		GP_DEBUG(1, "Malloc failed :(");
		goto err4;
	}

	font->max_glyph_width = 0;
	font->max_glyph_advance = 0;
	font->ascend  = 0;
	font->descend = 0;

	uint64_t avg_advance = 0;

	for (i = 0x20; i < 0x7f; i++) {
		GP_DEBUG(4, "Loading and rendering glyph '%c'", i);

		if (load_and_render_glyph(priv->face, i))
			goto err5;

		gp_glyph *glyph_bitmap = gp_get_glyph(font, i);

		copy_glyph(priv->face, glyph_bitmap);

		avg_advance += priv->face->glyph->advance.x;

		int16_t width = glyph_bitmap->bearing_x + glyph_bitmap->width;

		if (font->max_glyph_advance < glyph_bitmap->advance_x)
			font->max_glyph_advance = glyph_bitmap->advance_x;

		if (font->max_glyph_width < width)
			font->max_glyph_width = width;
	}

	font->ascend = (priv->face->size->metrics.ascender + 32) >> 6;
	font->descend = ((priv->face->size->metrics.height + 32) >> 6) - font->ascend;

	avg_advance = (((avg_advance + 32)>>6) + 47) / 95;
	font->avg_glyph_advance = (avg_advance + (avg_advance+5)/10);

	return font;
err5:
	free(font->glyphs[0].glyphs);
err4:
	free(font->glyphs[0].offsets);
err3:
	free(font);
err2:
	FT_Done_Face(priv->face);
err1:
	FT_Done_FreeType(priv->library);
err0:
	free(priv);
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
