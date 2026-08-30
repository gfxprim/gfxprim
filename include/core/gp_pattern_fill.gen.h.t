@ include header.t
/*
 * Copyright (C) 2026 Cyril Hrubis <metan@ucw.cz>
 */
/**
 * @file gp_pattern_fill.gen.h
 * @brief Rows/columns of pattern pixels, one function per pixel pack.
 *
 * A pattern row is what a pattern looks like on one row of a pixmap. It is
 * built once by gp_pixel_pattern_row_init_*() and then written by
 * gp_write_pixels_pattern_*() as many times as needed, the tile being four
 * rows high a fill builds four of them rather than one per row of the pixmap.
 *
 * Two bytes hold a whole number of tiles for every pixel size a pattern fits
 * into, so a row is two bytes over and over, see #gp_pixel_pattern_row.
 *
 * A column of a pattern is four pixels that repeat, it is built by
 * gp_pixel_pattern_col_init_*() and written by gp_write_pixels_pattern_col().
 * These are inline, a vertical line is drawn in a single place.
 *
 * Both patterns in the pixel are applied, the one for the color channels and
 * the one for the alpha channel. A pixel that comes out fully transparent is
 * not drawn at all, so that a stipple draws over what is under it instead of
 * erasing it.
 */
#include <core/gp_pixmap.h>
#include <core/gp_get_put_pixel.gen.h>
#include <core/gp_pattern_fill.h>

@ for ps in pixelpacks:
@     if ps.size <= 4:
/**
 * @brief Builds a row of {{ ps.suffix }} pattern pixels.
 *
 * The pixmap is needed for its pixel type, the alpha channel has to be told
 * apart from the rest of the pixel, and for its offset.
 *
 * The row is built for runs that start at x, or at any column that is
 * a multiple of #GP_PIXEL_PATTERN_W away from it.
 *
 * @param self A row to fill in.
 * @param pixmap A pixmap the row is drawn into.
 * @param pixel A pattern pixel, see gp_pixel_pattern().
 * @param x A column the run of pixels starts at.
 * @param y A row the pixels are drawn at.
 */
void gp_pixel_pattern_row_init_{{ ps.suffix }}(gp_pixel_pattern_row *self,
                                     const gp_pixmap *pixmap, gp_pixel pixel,
                                     gp_coord x, gp_coord y);

/**
 * @brief Builds a column of {{ ps.suffix }} pattern pixels.
 *
 * @param self A column to fill in.
 * @param pixmap A pixmap the column is drawn into.
 * @param pixel A pattern pixel, see gp_pixel_pattern().
 * @param x A column the pixels are drawn at.
 */
static inline void gp_pixel_pattern_col_init_{{ ps.suffix }}(gp_pixel_pattern_col *self,
                                     const gp_pixmap *pixmap, gp_pixel pixel,
                                     gp_coord x)
{
	const uint8_t *color = gp_pixel_pattern_lookup_table_get(gp_pixel_pattern_get(pixel));
	const uint8_t *alpha = gp_pixel_pattern_lookup_table_get(gp_pixel_pattern_alpha_get(pixel));
	gp_pixel amask = gp_pixel_alpha_mask(pixmap->pixel_type);
	gp_pixel fg = gp_pixel_pattern_fg_get(pixel);
	gp_pixel bg = gp_pixel_pattern_bg_get(pixel);
	unsigned int shift = GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(pixmap, x);
	uint8_t bit = 8 >> ((pixmap->offset + x) % GP_PIXEL_PATTERN_W);
	unsigned int i;

	for (i = 0; i < GP_PIXEL_PATTERN_H; i++) {
		gp_pixel px;

		px = ((color[i] & bit) ? fg : bg) & ~amask;
		px |= ((alpha[i] & bit) ? fg : bg) & amask;

		if (amask && !(px & amask)) {
			self->pixels[i] = 0;
			self->mask[i] = 0;
			continue;
		}

		self->pixels[i] = (px & {{ hex((1 << ps.size) - 1) }}) << shift;
		self->mask[i] = {{ hex((1 << ps.size) - 1) }} << shift;
	}
}

@     end
@ end
