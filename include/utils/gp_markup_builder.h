// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef UTILS_GP_MARKUP_BUILDER_H
#define UTILS_GP_MARKUP_BUILDER_H

#include <utils/gp_markup.h>

typedef struct gp_markup_builder {
	size_t glyph_cnt;
	gp_markup_glyph *glyphs;
} gp_markup_builder;

static inline void gp_markup_builder_glyph(gp_markup_builder *self,
                                           uint32_t glyph, uint8_t fmt, uint16_t fg_color)
{
	if (!self->glyphs)
		goto ret;

	self->glyphs[self->glyph_cnt].glyph = glyph;
	self->glyphs[self->glyph_cnt].fmt = fmt;
	self->glyphs[self->glyph_cnt].fg_color = fg_color;

ret:
	self->glyph_cnt++;
}

static inline void gp_markup_builder_space(gp_markup_builder *self, uint8_t fmt)
{
	if (!self->glyphs)
		goto ret;

	if (!self->glyph_cnt)
		return;

	if (self->glyphs[self->glyph_cnt-1].glyph == ' ' ||
	    self->glyphs[self->glyph_cnt-1].glyph == '\n')
		return;

	self->glyphs[self->glyph_cnt].glyph = ' ';
	self->glyphs[self->glyph_cnt].fmt = fmt;
	self->glyphs[self->glyph_cnt].space_padd = 0;

ret:
	self->glyph_cnt++;
}

static inline void gp_markup_builder_newline(gp_markup_builder *self)
{
	if (!self->glyphs)
		goto ret;

	self->glyphs[self->glyph_cnt].glyph = '\n';
	self->glyphs[self->glyph_cnt].fmt = 0;

ret:
	self->glyph_cnt++;
}

static inline void gp_markup_builder_hline(gp_markup_builder *self)
{
	if (!self->glyphs)
		goto ret;

	self->glyphs[self->glyph_cnt].glyph = '\n';
	self->glyphs[self->glyph_cnt].fmt = GP_MARKUP_STRIKE;

ret:
	self->glyph_cnt++;
}

static inline void gp_markup_builder_finish(gp_markup_builder *self)
{
	if (!self->glyphs)
		return;

	if (self->glyph_cnt && self->glyphs[self->glyph_cnt - 1].glyph == ' ')
		self->glyph_cnt--;

	self->glyphs[self->glyph_cnt].glyph = 0;
	self->glyphs[self->glyph_cnt].fmt = 0;
}

static inline gp_markup *gp_markup_builder_alloc(gp_markup_builder *self)
{
	gp_markup *ret = malloc(sizeof(gp_markup) + sizeof(gp_markup_glyph) * (self->glyph_cnt + 1));

	if (!ret)
		return NULL;

	self->glyphs = ret->glyphs;
	self->glyph_cnt = 0;

	return ret;
}

#endif /* UTILS_GP_MARKUP_BUILDER_H */
