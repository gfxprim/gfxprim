// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025-2026 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_pattern_fill.h
 * @brief A pattern fill constants.
 *
 * A pattern is counterpart to dithering used for drawing shapes in low bit
 * depths. Patterns alternate between two colors, foreground and background to
 * fill a shape.
 *
 * An alpha pattern applies on the top of the color or a color pattern and is
 * used to emulate transparency. The alpha pattern simply defines which pixels
 * are skipped from drawing, so instead of alternating between two colors, we
 * alternate between which pixels are painted and which are left untouched.
 */

#ifndef CORE_GP_PATTERN_FILL_H
#define CORE_GP_PATTERN_FILL_H

#include <core/gp_types.h>
#include <core/gp_pixel.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Pattern fill constants.
 *
 * The upper bits enable a pattern and the bottom bit changes the starting
 * offset, i.e. wheter pixel at even address starts as a black or white.
 *
 * @attention A pattern is carried in the pixel value itself, in the bits above
 *            the color, so it only works with pixel types of at most 4 bits.
 *            That is G1, G2, G4 and, with a pattern for the alpha channel of
 *            its own, GA11 and GA22.
 */
enum gp_pixel_pattern {
	/**
	 * @brief Solid foreground.
	 *
	 * If upper byte of the pixel is zeroed it's a solid color.
	 */
	GP_PIXEL_PATTERN_FG = 0x00,

	/**
	 * @brief This patterns pattern draws every other pixel and swaps
	 *        offset on each line.
	 *
	 * @code
	 * # # # # # # # #
	 *  # # # # # # #
	 * # # # # # # # #
	 *  # # # # # # #
	 * # # # # # # # #
	 *  # # # # # # #
	 * # # # # # # # #
	 * @endcode
	 *
	 * @attention This pattern value can be also passed to all hline and
	 *            vline functions where it will draw a dotted line that
	 *            matches the placement of white and black pixels in the
	 *            pattern.
	 */
	GP_PIXEL_PATTERN_50 = 0x01,

	/**
	 * @brief This pattern draws ascending 45 degrees lines that are
	 *        spaced so that the color fills 25% of the area.
	 *
	 * @code
	 * #   #   #   #   #
	 *    #   #   #   #
	 *   #   #   #   #
	 *  #   #   #   #
	 * #   #   #   #   #
	 * @endcode
	 *
	 * @attention This pattern value can be also passed to all hline and
	 *            vline functions where it will draw a dotted line that
	 *            matches the placement of white and black pixels in the
	 *            pattern.
	 */
	GP_PIXEL_PATTERN_ASC_25 = 0x02,

	/**
	 * @brief This pattern draws descending 45 degrees lines that are
	 *        spaced so that the color fills 25% of the area.
	 *
	 * @code
	 * #   #   #   #   #
	 *  #   #   #   #
	 *   #   #   #   #
	 *    #   #   #   #
	 * #   #   #   #   #
	 * @endcode
	 *
	 * @attention This pattern value can be also passed to all hline and
	 *            vline functions where it will draw a dotted line that
	 *            matches the placement of white and black pixels in the
	 *            pattern.
	 */
	GP_PIXEL_PATTERN_DSC_25 = 0x03,

	/**
	 * @brief This pattern draws ascending 45 degrees lines that are
	 *        spaced so that the color fills 75% of the area.
	 *
	 * @code
	 *  ### ### ### ### #
	 *  ## ### ### ### ##
	 *  # ### ### ### ###
	 *   ### ### ### ###
	 *  ### ### ### ### #
	 * @endcode
	 *
	 * @attention This pattern value can be also passed to all hline and
	 *            vline functions where it will draw a dotted line that
	 *            matches the placement of white and black pixels in the
	 *            pattern.
	 */
	GP_PIXEL_PATTERN_ASC_75 = 0x04,

	/**
	 * @brief This pattern draws ascending 45 degrees lines that are
	 *        spaced so that the color fills 75% of the area.
	 *
	 * @code
	 *  ### ### ### ### #
	 *   ### ### ### ###
	 *  # ### ### ### ###
	 *  ## ### ### ### ##
	 *  ### ### ### ### #
	 * @endcode
	 *
	 * @attention This pattern value can be also passed to all hline and
	 *            vline functions where it will draw a dotted line that
	 *            matches the placement of white and black pixels in the
	 *            pattern.
	 */
	GP_PIXEL_PATTERN_DSC_75 = 0x05,

	/**
	 * @brief This pattern draws dots that fill 12.5% of the space.
	 *
	 * @code
	 * #   #   #   #
	 *
	 *   #   #   #   #
	 *
	 * #   #   #   #
	 *
	 *   #   #   #   #
	 *
	 * @endcode
	 */
	GP_PIXEL_PATTERN_DOTS_12_5 = 0x06,

	/**
	 * @brief This pattern draws dots that fill 25% of the space.
	 *
	 * @code
	 *  # # # # # # # #
	 *
	 *  # # # # # # # #
	 *
	 *  # # # # # # # #
	 *
	 *  # # # # # # # #
	 *
	 * @endcode
	 */
	GP_PIXEL_PATTERN_DOTS_25 = 0x07,

	/**
	 * @brief This pattern draws inverse dots that fill 75% of the space.
	 *
	 * @code
	 *  # # # # # # # #
	 * ################
	 *  # # # # # # # #
	 * ################
	 *  # # # # # # # #
	 * ################
	 *  # # # # # # # #
	 * ################
	 * @endcode
	 */
	GP_PIXEL_PATTERN_DOTS_75 = 0x08,

	/**
	 * @brief This pattern draws inverse dots that fill 87.5% of the space.
	 *
	 * @code
	 *  ### ### ### ###
	 * ################
	 * ## ### ### ### #
	 * ################
	 *  ### ### ### ###
	 * ################
	 * ## ### ### ### #
	 * ################
	 * @endcode
	 */
	GP_PIXEL_PATTERN_DOTS_87_5 = 0x09,

	/**
	 * @brief This pattern draws dots that fill 6.25% of the space.
	 *
	 * A single dot per 4x4 tile, i.e. half of what
	 * #GP_PIXEL_PATTERN_DOTS_12_5 draws, and the lightest step of the
	 * gp_pixel_pattern_by_shade() ladder. Sparse enough that a small area
	 * of it reads as scattered dots rather than as a shade.
	 *
	 * @code
	 * #   #   #   #
	 *
	 *
	 *
	 * #   #   #   #
	 *
	 *
	 *
	 * @endcode
	 */
	GP_PIXEL_PATTERN_DOTS_6_25 = 0x0a,

	/**
	 * @brief This pattern draws inverse dots that fill 93.75% of the space.
	 *
	 * A single hole per 4x4 tile, i.e. the inverse of
	 * #GP_PIXEL_PATTERN_DOTS_6_25 and half of what
	 * #GP_PIXEL_PATTERN_DOTS_87_5 leaves empty, and the darkest step of
	 * the gp_pixel_pattern_by_shade() ladder. As with the pattern it
	 * inverts, a small area of it reads as a solid someone has poked holes
	 * into rather than as a shade.
	 *
	 * @code
	 *  ### ### ### ###
	 * ################
	 * ################
	 * ################
	 *  ### ### ### ###
	 * ################
	 * ################
	 * ################
	 * @endcode
	 */
	GP_PIXEL_PATTERN_DOTS_93_75 = 0x0b,

	/**
	 * @brief Solid background.
	 *
	 * Returned only by the color to pattern mapping function.
	 */
	GP_PIXEL_PATTERN_BG = 0x0c,
};

/**
 * @brief Number of patterns.
 */
#define GP_PIXEL_PATTERN_CNT 12

/**
 * @brief Pattern mask.
 *
 * After applying a pattern mask on a pixel the value can be compared
 * to the pattern constants.
 */
#define GP_PIXEL_PATTERN_MASK 0x0f00

/**
 * @brief Pattern shift.
 *
 * The pattern value is shifted to the left by this number of bits.
 */
#define GP_PIXEL_PATTERN_SHIFT 8

/**
 * @brief Alpha channel pattern mask.
 *
 * A pattern for the alpha channel, applied independently of the one in
 * #GP_PIXEL_PATTERN_MASK, which is applied to the color channels.
 *
 * Zero, i.e. #GP_PIXEL_PATTERN_FG, means that the alpha channel is taken
 * from the foreground color everywhere, so a pattern built without one behaves
 * as it always did.
 */
#define GP_PIXEL_PATTERN_ALPHA_MASK 0xf000

/**
 * @brief Alpha channel pattern shift.
 */
#define GP_PIXEL_PATTERN_ALPHA_SHIFT 12

/**
 * @brief Pattern foreground color mask.
 */
#define GP_PIXEL_PATTERN_FG_MASK 0x000f

/**
 * @brief Pattern foreground color shift.
 */
#define GP_PIXEL_PATTERN_FG_SHIFT 0

/**
 * @brief Pattern background color mask.
 */
#define GP_PIXEL_PATTERN_BG_MASK 0x00f0

/**
 * @brief Pattern background color shift.
 */
#define GP_PIXEL_PATTERN_BG_SHIFT 4

/**
 * @brief Returns a pattern for a grayscale pixel.
 *
 * @param pixel A 1bpp pixel.
 * @return A pattern constant.
 */
static inline enum gp_pixel_pattern gp_pixel_pattern_get(gp_pixel pixel)
{
	return (pixel & GP_PIXEL_PATTERN_MASK)>>GP_PIXEL_PATTERN_SHIFT;
}

/**
 * @brief Returns a pattern foreground for a grayscale pixel.
 *
 * @param pixel A 1bpp pixel.
 * @return A pixel foreground color.
 */
static inline gp_pixel gp_pixel_pattern_fg_get(gp_pixel pixel)
{
	return (pixel & GP_PIXEL_PATTERN_FG_MASK) >> GP_PIXEL_PATTERN_FG_SHIFT;
}

/**
 * @brief Returns a pattern background for a grayscale pixel.
 *
 * @param pixel A 1bpp pixel.
 * @return A pixel background color.
 */
static inline gp_pixel gp_pixel_pattern_bg_get(gp_pixel pixel)
{
	return (pixel & GP_PIXEL_PATTERN_BG_MASK) >> GP_PIXEL_PATTERN_BG_SHIFT;
}

/**
 * @brief Creates a pattern from a colors and pattern type.
 *
 * @param fg_color A pattern foreground color.
 * @param bg_color A pattern background color.
 * @param pattern A pattern.
 *
 * @return A pattern pixel color.
 */
static inline gp_pixel gp_pixel_pattern(uint8_t fg_color, uint8_t bg_color, enum gp_pixel_pattern pattern)
{
	if (fg_color == bg_color)
		return fg_color;

	return (GP_PIXEL_PATTERN_FG_MASK & (fg_color << GP_PIXEL_PATTERN_FG_SHIFT)) |
	       (GP_PIXEL_PATTERN_BG_MASK & (bg_color << GP_PIXEL_PATTERN_BG_SHIFT)) |
	       (GP_PIXEL_PATTERN_MASK & (pattern<<GP_PIXEL_PATTERN_SHIFT));
}

/**
 * @brief Returns an alpha channel pattern for a grayscale pixel.
 *
 * @param pixel A pixel with a pattern.
 * @return A pattern constant.
 */
static inline enum gp_pixel_pattern gp_pixel_pattern_alpha_get(gp_pixel pixel)
{
	return (pixel & GP_PIXEL_PATTERN_ALPHA_MASK)>>GP_PIXEL_PATTERN_ALPHA_SHIFT;
}

/**
 * @brief Returns true if a pixel carries a pattern.
 *
 * Meaningful only for pixel types of at most 4 bits per pixel, in the deeper
 * ones these bits are a part of the color.
 *
 * @param pixel A pixel.
 * @return Non-zero if the pixel has a color or an alpha pattern.
 */
static inline bool gp_pixel_is_pattern(gp_pixel pixel)
{
	return pixel & (GP_PIXEL_PATTERN_MASK | GP_PIXEL_PATTERN_ALPHA_MASK);
}

/**
 * @brief Creates a pattern that patterns the alpha channel separately.
 *
 * The color channels follow the pattern, the alpha channel follows the alpha
 * pattern, and each of them picks between the foreground and the background
 * color on its own. That is how a shade and a translucency can be drawn at
 * once on a pixel type that has a single bit for each: a 50% pattern with a
 * 25% alpha pattern is a mid gray that is a quarter opaque.
 *
 * With #GP_PIXEL_PATTERN_FG as the alpha pattern this is exactly
 * gp_pixel_pattern().
 *
 * @param fg_color A pattern foreground color.
 * @param bg_color A pattern background color.
 * @param color_pattern A pattern for the color channels.
 * @param alpha_pattern A pattern for the alpha channel.
 *
 * @return A pattern pixel color.
 */
static inline gp_pixel gp_pixel_pattern_alpha(uint8_t fg_color,
                                              uint8_t bg_color,
                                              enum gp_pixel_pattern color_pattern,
                                              enum gp_pixel_pattern alpha_pattern)
{
	gp_pixel pattern = gp_pixel_pattern(fg_color, bg_color, color_pattern);

	if (alpha_pattern == GP_PIXEL_PATTERN_FG)
		return pattern;

	return pattern | (GP_PIXEL_PATTERN_ALPHA_MASK &
	        (alpha_pattern << GP_PIXEL_PATTERN_ALPHA_SHIFT));
}

/**
 * @brief Pattern width.
 *
 * All patterns are limited to this width.
 */
#define GP_PIXEL_PATTERN_W 4

/**
 * @brief Pattern height.
 *
 * All patterns are limited to this height.
 */
#define GP_PIXEL_PATTERN_H 4

/**
 * @brief A pattern lookup tables.
 */
extern uint8_t *gp_pixel_pattern_lookup_table[GP_PIXEL_PATTERN_CNT];

/**
 * @brief Returns a pattern lookup table.
 *
 * @param pattern A pixel pattern.
 * @return The four tile rows of the pattern, one byte each.
 */
static inline uint8_t *gp_pixel_pattern_lookup_table_get(enum gp_pixel_pattern pattern)
{
	return gp_pixel_pattern_lookup_table[pattern];
}

/**
 * @brief Maps a foreground coverage to the nearest pattern.
 *
 * The steps are the background, 6.25%, 12.5%, 25%, 50%, 75%, 87.5%, 93.75%
 * and the foreground, and this is the nearest of those.
 *
 * @param density A foreground coverage, 0 for none and 255 for a full one.
 * @return A pattern, or two special values #GP_PIXEL_PATTERN_BG and
 *         #GP_PIXEL_PATTERN_FG at either end of the ladder.
 */
enum gp_pixel_pattern gp_pixel_pattern_by_shade(uint8_t density);

/**
 * @brief A row of a pattern.
 *
 * The tile is four pixels wide, so two bytes hold a whole number of tiles for
 * every pixel size a pattern fits into, and a row of a pattern is two bytes
 * repeated over and over. Up to two bits per pixel a single byte already holds
 * a whole number of tiles and the two bytes are the same.
 *
 * The upper byte is the one written into the byte a run starts in, which is
 * why the row is built for a column the run starts at.
 *
 * Built by gp_pixel_pattern_row_init_1BPP_DB() and friends, written by
 * gp_write_pixels_pattern_1BPP_DB() and friends.
 */
typedef struct gp_pixel_pattern_row {
	/** @brief Two bytes of pattern pixels the row repeats. */
	uint16_t pixels;
	/**
	 * @brief Which bits of the two bytes are written.
	 *
	 * The ones missing are the fully transparent pixels of the pattern,
	 * which are left alone rather than drawn.
	 */
	uint16_t mask;
} gp_pixel_pattern_row;

/**
 * @brief A column of a pattern.
 *
 * The tile is four rows high, so a column of a pattern is four pixels that
 * repeat. A pixel never spans two bytes at four bits per pixel or less, so
 * each of them is a byte, with the pixel already shifted to where the column
 * sits in it, and a mask of the bits it covers.
 *
 * A pixel that comes out fully transparent is not drawn at all rather than
 * written as a transparent pixel, its mask is zero. That is what makes an
 * alpha pattern a way of drawing a translucent shape over something instead
 * of a way of erasing what is under it. A pixel type with no alpha channel has
 * nothing to be transparent in, so nothing is ever skipped there.
 *
 * Built by gp_pixel_pattern_col_init_1BPP_DB() and friends, written by
 * gp_write_pixels_pattern_col().
 */
typedef struct gp_pixel_pattern_col {
	/** @brief Pattern pixels for the rows of the tile. */
	uint8_t pixels[GP_PIXEL_PATTERN_H];
	/** @brief Which bits of the byte each of the pixels covers. */
	uint8_t mask[GP_PIXEL_PATTERN_H];
} gp_pixel_pattern_col;

/**
 * @brief Writes a column of pattern pixels.
 *
 * @param start A pointer to the byte the first pixel is in.
 * @param bytes_per_row A distance between two rows in bytes.
 * @param cnt A number of pixels to write.
 * @param col A column of the pattern, see gp_pixel_pattern_col_init_1BPP_DB().
 * @param y A row of the first pixel.
 */
static inline void gp_write_pixels_pattern_col(void *start, size_t bytes_per_row,
                                               size_t cnt,
                                               const gp_pixel_pattern_col *col,
                                               gp_coord y)
{
	unsigned int i = y % GP_PIXEL_PATTERN_H;
	uint8_t *p = start;

	for (; cnt; cnt--, p += bytes_per_row) {
		*p = (*p & ~col->mask[i]) | col->pixels[i];
		i = (i + 1) % GP_PIXEL_PATTERN_H;
	}
}

#endif /* CORE_GP_PATTERN_FILL_H */
