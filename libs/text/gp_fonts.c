// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2021 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <text/gp_fonts.h>

extern const gp_font_face gp_default_font;

extern const gp_font_family font_family_gfxprim;
extern const gp_font_family font_family_haxor_medium_10;
extern const gp_font_family font_family_haxor_medium_11;
extern const gp_font_family font_family_haxor_medium_12;
extern const gp_font_family font_family_haxor_narrow_15;
extern const gp_font_family font_family_haxor_narrow_16;
extern const gp_font_family font_family_haxor_narrow_17;
extern const gp_font_family font_family_haxor_narrow_18;
extern const gp_font_family font_family_c64;
extern const gp_font_family font_family_haxor_tiny;
extern const gp_font_family font_family_square;

static const gp_font_family *const font_families[] = {
	&font_family_gfxprim,
	&font_family_haxor_medium_10,
	&font_family_haxor_medium_11,
	&font_family_haxor_medium_12,
	&font_family_haxor_narrow_15,
	&font_family_haxor_narrow_16,
	&font_family_haxor_narrow_17,
	&font_family_haxor_narrow_18,
	&font_family_c64,
	&font_family_haxor_tiny,
	&font_family_square,
};

#define FONT_FAMILIES_LAST_IDX (GP_ARRAY_SIZE(font_families) - 1)

const gp_font_family *gp_font_family_lookup(const char *family_name)
{
	unsigned int i;

	GP_DEBUG(3, "Looking for a font family '%s'", family_name);

	for (i = 0; i < GP_ARRAY_SIZE(font_families); i++) {
		if (!strcasecmp(font_families[i]->family_name, family_name))
			return font_families[i];
	}

	return NULL;
}

const gp_font_face *gp_font_family_face_lookup(const gp_font_family *family, uint8_t style)
{
	const gp_font_face *const *f;

	GP_DEBUG(3, "Looking for font style %s in family '%s'",
	         gp_font_style_name(style), family ? family->family_name : "NULL");

	if (!family)
		family = gp_font_family_default;

	for (f = family->fonts; *f; f++) {
		if ((*f)->style == GP_FONT_STYLE(style)) {
			GP_DEBUG(3, "Found style %i", style);
			return *f;
		}
	}

	if (style & GP_FONT_FALLBACK) {
		GP_DEBUG(3, "Style not found, choosing fallback");
		return family->fonts[0];
	}

	GP_DEBUG(3, "Style not found!");

	return NULL;
}

const gp_font_face *gp_font_face_lookup(const char *family_name, uint8_t style)
{
	const gp_font_family *family = gp_font_family_lookup(family_name);

	if (family)
		return gp_font_family_face_lookup(family, style);

	if (style & GP_FONT_FALLBACK) {
		GP_DEBUG(3, "Font family '%s' not found; using default font", family_name);
		return &gp_default_font;
	}

	GP_DEBUG(3, "Font family not found!");

	return NULL;
}

const gp_font_family *gp_fonts_iter_family(gp_fonts_iter *iter, int wrap,
                                           enum gp_fonts_iter_dir dir)
{
	switch (dir) {
	case GP_FONTS_ITER_NOP:
	break;
	case GP_FONTS_ITER_PREV:
		if (iter->family_idx - 1 < 0) {
			if (wrap)
				iter->family_idx = FONT_FAMILIES_LAST_IDX;
			else
				return NULL;
		} else {
			iter->family_idx--;
		}
	break;
	case GP_FONTS_ITER_NEXT:
		if (iter->family_idx + 1 > (int)FONT_FAMILIES_LAST_IDX) {
			if (wrap)
				iter->family_idx = 0;
			else
				return NULL;
		} else {
			iter->family_idx++;
		}
	break;
	case GP_FONTS_ITER_FIRST:
		iter->family_idx = 0;
	break;
	case GP_FONTS_ITER_LAST:
		iter->family_idx = FONT_FAMILIES_LAST_IDX;
	break;
	}

	if (dir != GP_FONTS_ITER_NOP)
		iter->font_idx = 0;

	return font_families[iter->family_idx];
}

static inline int last_font_idx(int family_idx)
{
	int i = 0;

	while (font_families[family_idx]->fonts[i])
		i++;

	return i - 1;
}

const gp_font_face *gp_fonts_iter_font(gp_fonts_iter *iter, int wrap,
                                       enum gp_fonts_iter_dir dir)
{
	switch (dir) {
	case GP_FONTS_ITER_NOP:
	break;
	case GP_FONTS_ITER_PREV:
		if (iter->font_idx - 1 < 0) {
			if (!gp_fonts_iter_family(iter, wrap, dir))
				return NULL;
			iter->font_idx = last_font_idx(iter->family_idx);
		} else {
			iter->font_idx--;
		}
	break;
	case GP_FONTS_ITER_NEXT:
		if (!font_families[iter->family_idx]->fonts[iter->font_idx + 1]) {
			if (!gp_fonts_iter_family(iter, wrap, dir))
				return NULL;
			iter->font_idx = 0;
		} else {
			iter->font_idx++;
		}
	break;
	case GP_FONTS_ITER_FIRST:
		iter->family_idx = 0;
		iter->font_idx = 0;
	break;
	case GP_FONTS_ITER_LAST:
		iter->family_idx = FONT_FAMILIES_LAST_IDX;
		iter->font_idx = last_font_idx(iter->family_idx);
	break;
	}

	return font_families[iter->family_idx]->fonts[iter->font_idx];
}
