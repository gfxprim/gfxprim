@ include source.t
/*
 * Text rendering rutines.
 *
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2020 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_mix_pixels.gen.h>

#include <gfx/gp_hline.h>

#include <text/gp_text_style.h>
#include <text/gp_font.h>
#include <text/gp_text.h>

#define WIDTH_TO_1BPP_BPP(width) ((width)/8 + ((width)%8 != 0))

static int get_width(const gp_text_style *style, int width)
{
	return width * style->pixel_xmul + (width - 1) * style->pixel_xspace;
}

@ for pt in pixeltypes:
@     if not pt.is_unknown():

static void text_draw_1BPP_{{ pt.name }}(gp_pixmap *pixmap, const gp_text_style *style,
                                         uint8_t bearing, gp_coord x, gp_coord y,
				         gp_pixel fg, const char *str, size_t max_chars)
{
	const char *p;
	gp_coord y0 = y;

	for (p = str; *p != '\0' && (size_t)(p - str) < max_chars; p++) {
		const gp_glyph *glyph = gp_get_glyph(style->font, *p);

		if (glyph == NULL)
			glyph = gp_get_glyph(style->font, ' ');

		int i, j, k, l;

		unsigned int x_mul = style->pixel_xmul + style->pixel_xspace;
		unsigned int y_mul = style->pixel_ymul + style->pixel_yspace;

		unsigned int bpp = WIDTH_TO_1BPP_BPP(glyph->width);

		y = y0;

		for (j = 0; j < glyph->height; j++) {
			for (i = 0; i < glyph->width; i++) {
				uint8_t bit = (glyph->bitmap[i/8 + j * bpp]) & (0x80>>(i%8));

				if (!bit)
					continue;

				int start_x = x + (i + glyph->bearing_x) * x_mul;

				if (!bearing && p == str)
					start_x -= glyph->bearing_x * x_mul;

				int start_y = y - (glyph->bearing_y - style->font->ascend) * y_mul;

				for (k = start_y; k < start_y + style->pixel_ymul; k++) {
					for (l = start_x; l < start_x + style->pixel_xmul; l++) {
						int px = l;
						int py = k;
						GP_TRANSFORM_POINT(pixmap, px, py);
						if (!GP_PIXEL_IS_CLIPPED(pixmap, px, py))
							gp_putpixel_raw_{{ pt.pixelsize.suffix }}(pixmap, px, py, fg);
					}
				}
			}

			y += style->pixel_ymul + style->pixel_yspace;
		}

		x += get_width(style, glyph->advance_x) + style->char_xspace;

		if (!bearing && p == str)
			x -= get_width(style, glyph->bearing_x);
	}
}

@ end

static void text_draw_1BPP(gp_pixmap *pixmap, const gp_text_style *style,
                           uint8_t bearing, int x, int y,
                           gp_pixel fg, const char *str, size_t max_chars)
{
	switch (pixmap->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		text_draw_1BPP_{{ pt.name }}(pixmap, style, bearing, x, y, fg, str, max_chars);
	break;
@ end
	default:
		GP_ABORT("Invalid pixmap->pixel_type");
	}
}

@ def text_8BPP(pt, use_bg):
	const char *p;

	gp_coord y0 = y;

	for (p = str; *p != '\0' && (size_t)(p - str) < max_chars; p++) {
		const gp_glyph *glyph = gp_get_glyph(style->font, *p);

		if (glyph == NULL)
			glyph = gp_get_glyph(style->font, ' ');

		int i, j, k;

		unsigned int x_mul = style->pixel_xmul + style->pixel_xspace;
		unsigned int y_mul = style->pixel_ymul + style->pixel_yspace;

		y = y0;

		for (j = 0; j < glyph->height; j++) {
			for (i = 0; i < glyph->width; i++) {
				uint8_t gray = glyph->bitmap[i + j * glyph->width];

				unsigned int x_start = x + (i + glyph->bearing_x) * x_mul;

				if (!bearing && p == str)
					x_start -= glyph->bearing_x * x_mul;

				if (!gray)
					continue;

				int cur_y = y - (glyph->bearing_y - style->font->ascend) * y_mul;

				for (k = 0; k < style->pixel_ymul; k++) {
@     if use_bg:
					gp_hline(pixmap, x_start, x_start + style->pixel_xmul - 1, cur_y + k,
					GP_MIX_PIXELS_{{ pt.name }}(fg, bg, gray));
@     else:
					unsigned int l;

					for (l = x_start; l < x_start + style->pixel_xmul; l++) {
						unsigned int px = l;
						unsigned int py = cur_y + k;
						//TODO: optimize this
						GP_TRANSFORM_POINT(pixmap, px, py);
						gp_mix_pixel_raw_clipped_{{ pt.name }}(pixmap, px, py, fg, gray);
					}
@     end
				}
			}

			y += style->pixel_ymul + style->pixel_yspace;
		}

		x += get_width(style, glyph->advance_x) + style->char_xspace;

		if (!bearing && p == str)
			x -= get_width(style, glyph->bearing_x);
	}
@ end
@
@ for pt in pixeltypes:
@     if not pt.is_unknown():

static void text_8BPP_bg_{{ pt.name }}(gp_pixmap *pixmap, const gp_text_style *style,
                                       uint8_t bearing, gp_coord x, gp_coord y,
				       gp_pixel fg, gp_pixel bg,
                                       const char *str, size_t max_chars)
{
@         text_8BPP(pt, True)
}

static void text_8BPP_{{ pt.name }}(gp_pixmap *pixmap, const gp_text_style *style,
                                    uint8_t bearing, gp_coord x, gp_coord y,
				    gp_pixel fg, const char *str, size_t max_chars)
{
@         text_8BPP(pt, False)
}

@ end

static void text_8BPP_bg(gp_pixmap *pixmap, const gp_text_style *style,
                         uint8_t bearing, gp_coord x, gp_coord y,
                         gp_pixel fg, gp_pixel bg,
                         const char *str, size_t max_chars)
{
	switch (pixmap->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		text_8BPP_bg_{{ pt.name }}(pixmap, style, bearing, x, y, fg, bg, str, max_chars);
	break;
@ end
	default:
		GP_ABORT("Invalid pixmap->pixel_type");
	}
}

static void text_8BPP(gp_pixmap *pixmap, const gp_text_style *style,
                      uint8_t bearing, gp_coord x, gp_coord y,
                      gp_pixel fg, const char *str, size_t max_chars)
{
	switch (pixmap->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		text_8BPP_{{ pt.name }}(pixmap, style, bearing, x, y, fg, str, max_chars);
	break;
@ end
	default:
		GP_ABORT("Invalid pixmap->pixel_type");
	}
}

void gp_text_raw(gp_pixmap *pixmap, const gp_text_style *style,
                 gp_coord x, gp_coord y, uint8_t flags,
                 gp_pixel fg, gp_pixel bg,
		 const char *str, size_t max_chars)
{
	uint8_t bearing = flags & GP_TEXT_BEARING;

	switch (style->font->glyph_bitmap_format) {
	case GP_FONT_BITMAP_1BPP:
		text_draw_1BPP(pixmap, style, bearing, x, y, fg, str, max_chars);
	break;
	case GP_FONT_BITMAP_8BPP:
		if (flags & GP_TEXT_NOBG)
			text_8BPP(pixmap, style, bearing, x, y, fg, str, max_chars);
		else
			text_8BPP_bg(pixmap, style, bearing, x, y, fg, bg, str, max_chars);
	break;
	default:
		GP_ABORT("Invalid font glyph bitmap format");
	}
}
