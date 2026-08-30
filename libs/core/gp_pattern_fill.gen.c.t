@ include source.t
/*
 * Rows of pattern pixels, one function per pixel pack.
 *
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_pattern_fill.gen.h>

@ def spread(ps, tile):
@     bits = 16
@     ppb = 8 // ps.size
@     res = 0
@     for x in range(0, bits // ps.size):
@         if tile & (8 >> (x % 4)):
@             i = x % ppb
@             shift = 8 - (i + 1) * ps.size if ps.bit_order == 'UB' else i * ps.size
@             shift += bits - 8 * (x // ppb + 1)
@             res |= ((1 << ps.size) - 1) << shift
@     end
@     return res
@ end
@
@ for ps in pixelpacks:
@     if ps.size <= 4:
@         pmask = (1 << ps.size) - 1
@         rep = 0xffff // pmask
/*
 * A tile row spread into the fields of the pixels: all ones where the tile has
 * the pixel, all zeroes where it does not.
 */
static const uint16_t spread_{{ ps.suffix }}[16] = {
	{{ ', '.join(['0x%04x' % spread(ps, t) for t in range(0, 16)]) }}
};

/*
 * The whole row at once: a tile row spread into the fields, and the two colors
 * repeated into every field, so that picking between them is three bit
 * operations rather than a walk of the pixels.
 */
void gp_pixel_pattern_row_init_{{ ps.suffix }}(gp_pixel_pattern_row *self,
                                     const gp_pixmap *pixmap, gp_pixel pixel,
                                     gp_coord x, gp_coord y)
{
	uint8_t y_mod = y % GP_PIXEL_PATTERN_H;
	gp_pixel pfg = gp_pixel_pattern_fg_get(pixel);
	gp_pixel pbg = gp_pixel_pattern_bg_get(pixel);
	gp_pixel pamask = gp_pixel_alpha_mask(pixmap->pixel_type);
	uint16_t color, alpha, fg, bg, amask;

	color = gp_pixel_pattern_lookup_table_get(gp_pixel_pattern_get(pixel))[y_mod];
	alpha = gp_pixel_pattern_lookup_table_get(gp_pixel_pattern_alpha_get(pixel))[y_mod];

	color = spread_{{ ps.suffix }}[color & 0x0f];
	alpha = spread_{{ ps.suffix }}[alpha & 0x0f];

	fg = (pfg & {{ hex(pmask) }}) * {{ hex(rep) }};
	bg = (pbg & {{ hex(pmask) }}) * {{ hex(rep) }};
	amask = (pamask & {{ hex(pmask) }}) * {{ hex(rep) }};

	/*
	 * Which pixels are written: all of them where there is no alpha
	 * channel, otherwise the ones whose alpha does not come out zero.
	 */
	if (!amask)
		self->mask = 0xffff;
	else
		self->mask = ((pfg & pamask) ? alpha : 0) |
		             ((pbg & pamask) ? (uint16_t)~alpha : 0);

	self->pixels = (((color & fg) | (~color & bg)) & ~amask) |
	               (((alpha & fg) | (~alpha & bg)) & amask);
@         if ps.size <= 2:

	/* Both bytes are the same, so the row starts anywhere */
	(void) x;
@         else:

	/*
	 * The upper byte is written first, it holds the first two pixels of
	 * the tile, so a run that starts in the second half of a tile starts
	 * with the lower one.
	 */
	if (((pixmap->offset + x) / 2) % 2) {
		self->pixels = (self->pixels << 8) | (self->pixels >> 8);
		self->mask = (self->mask << 8) | (self->mask >> 8);
	}
@         end
}

@     end
@ end
