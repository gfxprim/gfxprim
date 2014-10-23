@ include source.t
/*
 * Text rendering rutines.
 *
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_GetPutPixel.h"
#include "core/GP_MixPixels.gen.h"
#include "gfx/GP_HLine.h"
#include "GP_TextStyle.h"
#include "GP_Font.h"

#define WIDTH_TO_1BPP_BPP(width) ((width)/8 + ((width)%8 != 0))

static int get_width(GP_TextStyle *style, int width)
{
	return width * style->pixel_xmul + (width - 1) * style->pixel_xspace;
}

@ for pt in pixeltypes:
@     if not pt.is_unknown():

static void text_draw_1BPP_{{ pt.name }}(GP_Context *context, GP_TextStyle *style,
                                         GP_Coord x, GP_Coord y,
				         GP_Pixel fg, const char *str)
{
	const char *p;

	GP_Coord y0 = y;

	for (p = str; *p != '\0'; p++) {
		const GP_GlyphBitmap *glyph = GP_GetGlyphBitmap(style->font, *p);

		if (glyph == NULL)
			glyph = GP_GetGlyphBitmap(style->font, ' ');

		int i, j, k;

		unsigned int x_mul = style->pixel_xmul + style->pixel_xspace;
		unsigned int y_mul = style->pixel_ymul + style->pixel_yspace;

		unsigned int bpp = WIDTH_TO_1BPP_BPP(glyph->width);

		y = y0;

		for (j = 0; j < glyph->height; j++) {
			for (i = 0; i < glyph->width; i++) {
				uint8_t bit = (glyph->bitmap[i/8 + j * bpp]) & (0x80>>(i%8));

				unsigned int x_start = x + (i + glyph->bearing_x) * x_mul;

				if (p == str)
					x_start -= glyph->bearing_x * x_mul;

				if (!bit)
					continue;

				for (k = 0; k < style->pixel_ymul; k++)
					GP_HLine(context, x_start, x_start + style->pixel_xmul - 1,
					         y - (glyph->bearing_y - style->font->ascend) * y_mul + k, fg);
			}

			y += style->pixel_ymul + style->pixel_yspace;
		}

		x += get_width(style, glyph->advance_x) + style->char_xspace;

		if (p == str)
			x -= get_width(style, glyph->bearing_x);
	}
}

@ end

static void text_draw_1BPP(GP_Context *context, GP_TextStyle *style, int x, int y,
                           GP_Pixel fg, const char *str)
{
	switch (context->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		text_draw_1BPP_{{ pt.name }}(context, style, x, y, fg, str);
	break;
@ end
	default:
		GP_ABORT("Invalid context->pixel_type");
	}
}

@ def text_8BPP(pt, use_bg):
	const char *p;

	GP_Coord y0 = y;

	for (p = str; *p != '\0'; p++) {
		const GP_GlyphBitmap *glyph = GP_GetGlyphBitmap(style->font, *p);

		if (glyph == NULL)
			glyph = GP_GetGlyphBitmap(style->font, ' ');

		int i, j, k;

		unsigned int x_mul = style->pixel_xmul + style->pixel_xspace;
		unsigned int y_mul = style->pixel_ymul + style->pixel_yspace;

		y = y0;

		for (j = 0; j < glyph->height; j++) {
			for (i = 0; i < glyph->width; i++) {
				uint8_t gray = glyph->bitmap[i + j * glyph->width];

				unsigned int x_start = x + (i + glyph->bearing_x) * x_mul;

				if (p == str)
					x_start -= glyph->bearing_x * x_mul;

				if (!gray)
					continue;

				int cur_y = y - (glyph->bearing_y - style->font->ascend) * y_mul;

				for (k = 0; k < style->pixel_ymul; k++) {
@     if use_bg:
					GP_HLine(context, x_start, x_start + style->pixel_xmul - 1, cur_y + k,
					GP_MIX_PIXELS_{{ pt.name }}(fg, bg, gray));
@     else:
					unsigned int l;

					for (l = x_start; l < x_start + style->pixel_xmul; l++) {
						unsigned int px = l;
						unsigned int py = cur_y + k;
						//TODO: optimize this
						GP_TRANSFORM_POINT(context, px, py);
						GP_MixPixel_Raw_Clipped_{{ pt.name }}(context, px, py, fg, gray);
					}
@     end
				}
			}

			y += style->pixel_ymul + style->pixel_yspace;
		}

		x += get_width(style, glyph->advance_x) + style->char_xspace;

		if (p == str)
			x -= get_width(style, glyph->bearing_x);
	}
@ end
@
@ for pt in pixeltypes:
@     if not pt.is_unknown():

static void text_8BPP_bg_{{ pt.name }}(GP_Context *context, GP_TextStyle *style,
                                       GP_Coord x, GP_Coord y,
				       GP_Pixel fg, GP_Pixel bg, const char *str)
{
@         text_8BPP(pt, True)
}

static void text_8BPP_{{ pt.name }}(GP_Context *context, GP_TextStyle *style,
                                    GP_Coord x, GP_Coord y,
				    GP_Pixel fg, const char *str)
{
@         text_8BPP(pt, False)
}

@ end

static void text_8BPP_bg(GP_Context *context, GP_TextStyle *style,
                         GP_Coord x, GP_Coord y,
                         GP_Pixel fg, GP_Pixel bg, const char *str)
{
	switch (context->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		text_8BPP_bg_{{ pt.name }}(context, style, x, y, fg, bg, str);
	break;
@ end
	default:
		GP_ABORT("Invalid context->pixel_type");
	}
}

static void text_8BPP(GP_Context *context, GP_TextStyle *style,
                      GP_Coord x, GP_Coord y,
                      GP_Pixel fg, const char *str)
{
	switch (context->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		text_8BPP_{{ pt.name }}(context, style, x, y, fg, str);
	break;
@ end
	default:
		GP_ABORT("Invalid context->pixel_type");
	}
}

void GP_Text_Raw(GP_Context *context, GP_TextStyle *style,
                 GP_Coord x, GP_Coord y, uint8_t flags,
                 GP_Pixel fg, GP_Pixel bg, const char *str)
{
	switch (style->font->glyph_bitmap_format) {
	case GP_FONT_BITMAP_1BPP:
		text_draw_1BPP(context, style, x, y, fg, str);
	break;
	case GP_FONT_BITMAP_8BPP:
		if (flags)
			text_8BPP(context, style, x, y, fg, str);
		else
			text_8BPP_bg(context, style, x, y, fg, bg, str);
	break;
	default:
		GP_ABORT("Invalid font glyph bitmap format");
	}
}
