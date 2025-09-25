@ include source.t
/*
 * Text rendering rutines.
 *
 * Copyright (C) 2009-2011 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_get_put_pixel.h>
#include <core/gp_mix_pixels.gen.h>
#include <core/gp_transform.h>

#include <gfx/gp_hline.h>

#include <utils/gp_utf.h>

#include <text/gp_text_style.h>
#include <text/gp_font.h>
#include <text/gp_text.h>

#define WIDTH_TO_1BPP_BPP(width) ((width)/8 + ((width)%8 != 0))

static int get_width(const gp_text_style *style, int width)
{
	return width * style->pixel_xmul + (width - 1) * style->pixel_xspace;
}

static uint16_t bit_lookup[] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
};

@ for pt in pixeltypes:
@     if not pt.is_unknown():

static void draw_1BPP_glyph_{{ pt.name }}(gp_pixmap *pixmap, const gp_text_style *style,
                                          gp_coord x, gp_coord y, gp_pixel fg, const gp_glyph *glyph)
{
	int i, j, k;

	unsigned int bpp = WIDTH_TO_1BPP_BPP(glyph->width);

	unsigned int x_mul = style->pixel_xmul + style->pixel_xspace;
	unsigned int y_mul = style->pixel_ymul + style->pixel_yspace;

	x += glyph->bearing_x * x_mul;
	y -= (glyph->bearing_y - gp_font_ascent(style->font)) * y_mul;

	for (j = 0; j < glyph->height; j++) {
		for (i = 0; i < glyph->width; i++) {
			uint8_t bit = (glyph->bitmap[i/8 + j * bpp]) & bit_lookup[i%8];

			if (!bit)
				continue;

			gp_coord px = x + i * x_mul;
			gp_coord py = y;
			gp_coord sx = style->pixel_xmul;
			gp_coord sy = style->pixel_ymul;

			GP_TRANSFORM_RECT(pixmap, px, py, sx, sy);

			for (k = py; k < py + sy; k++)
				gp_hline_raw_{{ pt.pixelpack.suffix }}(pixmap, px, px + sx, k, fg);
		}
		y += y_mul;
	}
}

static void draw_1BPP_glyph_nomul_{{ pt.name }}(gp_pixmap *pixmap, const gp_text_style *style,
                                                gp_coord x, gp_coord y, gp_pixel fg, const gp_glyph *glyph)
{
	int i, j;

	unsigned int bpp = WIDTH_TO_1BPP_BPP(glyph->width);

	x += glyph->bearing_x;
	y -= (glyph->bearing_y - gp_font_ascent(style->font));

	for (j = 0; j < glyph->height; j++) {
		for (i = 0; i < glyph->width; i++) {
			uint8_t bit = (glyph->bitmap[i/8 + j * bpp]) & bit_lookup[i%8];

			if (!bit)
				continue;

			gp_coord px = x+i;
			gp_coord py = y;

			GP_TRANSFORM_POINT(pixmap, px, py);

			if (!GP_PIXEL_IS_CLIPPED(pixmap, px, py))
				gp_putpixel_raw_{{ pt.pixelpack.suffix }}(pixmap, px, py, fg);
		}
		y++;
	}
}

static void text_draw_1BPP_{{ pt.name }}(gp_pixmap *pixmap, const gp_text_style *style,
                                         uint8_t bearing, gp_coord x, gp_coord y,
				         gp_pixel fg, const char *str, size_t max_chars)
{
	uint32_t ch;
	size_t pos;

	unsigned int x_mul = style->pixel_xmul + style->pixel_xspace;

	int nomul = style->pixel_xmul == 1 && style->pixel_ymul == 1 &&
                    style->pixel_xspace == 0 && style->pixel_yspace == 0;

	for (pos = 0; pos < max_chars && (ch = gp_utf8_next(&str)); pos++) {
		const gp_glyph *glyph = gp_glyph_get(style->font, ch);

		gp_coord gx = x;

		if (!bearing && !pos)
			gx -= glyph->bearing_x * x_mul;

		if (nomul)
			draw_1BPP_glyph_nomul_{{ pt.name }}(pixmap, style, gx, y, fg, glyph);
		else
			draw_1BPP_glyph_{{ pt.name }}(pixmap, style, gx, y, fg, glyph);

		x += get_width(style, glyph->advance_x) + style->char_xspace;

		if (!bearing && !pos)
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

@ def glyph_8BPP(pt, bg):
static void draw_8BPP_glyph{{ bg }}_{{ pt.name }}(gp_pixmap *pixmap, const gp_text_style *style,
                                          gp_coord x, gp_coord y,
					  gp_pixel fg, gp_pixel bg,
					  const gp_glyph *glyph)
{
	gp_coord i, j, k;

@     if bg != '_bg':
	(void) bg;
@     end

	unsigned int x_mul = style->pixel_xmul + style->pixel_xspace;
	unsigned int y_mul = style->pixel_ymul + style->pixel_yspace;

	x += glyph->bearing_x * x_mul;
	y -= (glyph->bearing_y - gp_font_ascent(style->font)) * y_mul;

	gp_coord x0 = x;

	for (j = 0; j < glyph->height; j++) {
		for (i = 0; i < glyph->width; i++) {
			uint8_t gray = glyph->bitmap[i + j * glyph->width];

			if (!gray) {
				x += x_mul;
				continue;
			}

			for (k = 0; k < style->pixel_ymul; k++) {
@     if bg == '_bg':
					gp_hline(pixmap, x, x + style->pixel_xmul - 1, y + k,
					GP_MIX_PIXELS_{{ pt.name }}(fg, bg, gray));
@     else:
					gp_coord l;

					for (l = x; l < x + style->pixel_xmul; l++) {
						unsigned int px = l;
						unsigned int py = y + k;
						//TODO: optimize this
						GP_TRANSFORM_POINT(pixmap, px, py);
						gp_mix_pixel_raw_clipped_{{ pt.name }}(pixmap, px, py, fg, gray);
					}
@     end
			}
			x += x_mul;
		}

		x = x0;
		y += style->pixel_ymul + style->pixel_yspace;
	}
}
@ end

@ def text_8BPP(pt, bg):
	uint32_t ch;
	size_t pos;

	unsigned int x_mul = style->pixel_xmul + style->pixel_xspace;

	for (pos = 0; pos < max_chars && (ch = gp_utf8_next(&str)); pos++) {
		const gp_glyph *glyph = gp_glyph_get(style->font, ch);
		gp_coord gx = x;

		if (!bearing && !pos)
			gx -= glyph->bearing_x * x_mul;

		draw_8BPP_glyph{{ bg }}_{{ pt.name }}(pixmap, style, gx, y, fg, bg, glyph);

		x += get_width(style, glyph->advance_x) + style->char_xspace;

		if (!bearing && !pos)
			x -= get_width(style, glyph->bearing_x);
	}
@ end
@
@ for pt in pixeltypes:
@     if not pt.is_unknown():

@         glyph_8BPP(pt, "_bg")

@         glyph_8BPP(pt, "")

static void text_8BPP_bg_{{ pt.name }}(gp_pixmap *pixmap, const gp_text_style *style,
                                       uint8_t bearing, gp_coord x, gp_coord y,
				       gp_pixel fg, gp_pixel bg,
                                       const char *str, size_t max_chars)
{
@         text_8BPP(pt, "_bg")
}

static void text_8BPP_{{ pt.name }}(gp_pixmap *pixmap, const gp_text_style *style,
                                    uint8_t bearing, gp_coord x, gp_coord y,
				    gp_pixel fg, gp_pixel bg,
				    const char *str, size_t max_chars)
{
@         text_8BPP(pt, "")
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
                      gp_pixel fg, gp_pixel bg, const char *str, size_t max_chars)
{
	switch (pixmap->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		text_8BPP_{{ pt.name }}(pixmap, style, bearing, x, y, fg, bg, str, max_chars);
	break;
@ end
	default:
		GP_ABORT("Invalid pixmap->pixel_type");
	}
}

gp_size gp_text_raw(gp_pixmap *pixmap, const gp_text_style *style,
                    gp_coord x, gp_coord y, gp_text_flags flags,
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
			text_8BPP(pixmap, style, bearing, x, y, fg, bg, str, max_chars);
		else
			text_8BPP_bg(pixmap, style, bearing, x, y, fg, bg, str, max_chars);
	break;
	default:
		GP_ABORT("Invalid font glyph bitmap format");
	}

	return gp_text_width_len(style, GP_TEXT_LEN_BBOX, str, max_chars);
}

static void glyph_1BPP(gp_pixmap *pixmap, const gp_text_style *style,
                       gp_coord x, gp_coord y,
                       gp_pixel fg, const gp_glyph *glyph)
{
	int nomul = style->pixel_xmul == 1 && style->pixel_ymul == 1 &&
                    style->pixel_xspace == 0 && style->pixel_yspace == 0;

	switch (pixmap->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		if (nomul)
			draw_1BPP_glyph_nomul_{{ pt.name }}(pixmap, style, x, y, fg, glyph);
		else
			draw_1BPP_glyph_{{ pt.name }}(pixmap, style, x, y, fg, glyph);
	break;
@ end
	default:
		GP_ABORT("Invalid pixmap->pixel_type");
	}
}

static void glyph_8BPP(gp_pixmap *pixmap, const gp_text_style *style,
                       gp_coord x, gp_coord y,
                       gp_pixel fg, gp_pixel bg, const gp_glyph *glyph)
{
	switch (pixmap->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		draw_8BPP_glyph_{{ pt.name }}(pixmap, style, x, y, fg, bg, glyph);
	break;
@ end
	default:
		GP_ABORT("Invalid pixmap->pixel_type");
	}
}

static void glyph_8BPP_bg(gp_pixmap *pixmap, const gp_text_style *style,
                          gp_coord x, gp_coord y,
                          gp_pixel fg, gp_pixel bg, const gp_glyph *glyph)
{
	switch (pixmap->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown():
	case GP_PIXEL_{{ pt.name }}:
		draw_8BPP_glyph_bg_{{ pt.name }}(pixmap, style, x, y, fg, bg, glyph);
	break;
@ end
	default:
		GP_ABORT("Invalid pixmap->pixel_type");
	}
}

gp_size gp_glyph_draw(gp_pixmap *pixmap, const gp_text_style *style,
                      gp_coord x, gp_coord y, int flags,
                      gp_pixel fg_color, gp_pixel bg_color,
                      uint32_t glyph)
{
	uint8_t bearing = flags & GP_TEXT_BEARING;

	const gp_glyph *g = gp_glyph_get(style->font, glyph);

	gp_coord gx = x;
	gp_coord bearing_x = 0;

	switch (flags & 0x70) {
	case GP_VALIGN_ABOVE:
		y = y - gp_text_height(style) + 1;
	break;
	case GP_VALIGN_CENTER:
		y = y - gp_text_height(style)/2;
	break;
	case GP_VALIGN_BASELINE:
		y = y - gp_text_ascent(style) + 1;
	break;
	case GP_VALIGN_BELOW:
	break;
	}

	if (!bearing) {
		unsigned int x_mul = style->pixel_xmul + style->pixel_xspace;
		bearing_x = g->bearing_x * x_mul;
		gx -= bearing_x;
	}

	switch (style->font->glyph_bitmap_format) {
	case GP_FONT_BITMAP_1BPP:
		glyph_1BPP(pixmap, style, gx, y, fg_color, g);
	break;
	case GP_FONT_BITMAP_8BPP:
		if (flags & GP_TEXT_NOBG)
			glyph_8BPP(pixmap, style, gx, y, fg_color, bg_color, g);
		else
			glyph_8BPP_bg(pixmap, style, gx, y, fg_color, bg_color, g);
	break;
	default:
		GP_ABORT("Invalid font glyph bitmap format");
	}

	return gp_glyph_advance_x(style, glyph) + style->char_xspace - (!bearing ? bearing_x : 0);
}
