// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_pattern_fill.h
 * @brief A pattern fill constants.
 *
 * Constants for different fill patterns for 1bpp fill runtines.
 */

#ifndef CORE_GP_PATTERN_FILL_H
#define CORE_GP_PATTERN_FILL_H

#include <core/gp_types.h>
#include <stdbool.h>

/**
 * @brief Pattern fill constants.
 *
 * The upper bits enable a pattern and the bottom bit changes the starting
 * offset, i.e. wheter pixel at even address starts as a black or white.
 *
 * @attention Pattern fill works only with 1bpp aka monochrome colors.
 */
enum gp_pixel_pattern {
	/**
	 * @brief No pattern.
	 *
	 * If upper byte of the pixel is zeroed it's a solid color.
	 */
	GP_PIXEL_PATTERN_NONE = 0x00,

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
	 * #   #    #    #
	 *
	 *
	 *
	 * #   #    #    #
	 *
	 *
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
	 * ################
	 * ################
	 *  ### ### ### ###
	 * ################
	 * ################
	 * ################
	 * @endcode
	 */
	GP_PIXEL_PATTERN_DOTS_87_5 = 0x09,
};

/**
 * @brief Number of patterns.
 */
#define GP_PIXEL_PATTERN_CNT 10

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
 * @pixel A 1bpp pixel.
 * @return A pattern constant.
 */
static inline enum gp_pixel_pattern gp_pixel_pattern_get(gp_pixel pixel)
{
	return (pixel & GP_PIXEL_PATTERN_MASK)>>GP_PIXEL_PATTERN_SHIFT;
}

/**
 * @brief Returns a pattern foreground for a grayscale pixel.
 *
 * @pixel A 1bpp pixel.
 * @return A pixel foreground color.
 */
static inline gp_pixel gp_pixel_pattern_fg_get(gp_pixel pixel)
{
	return (pixel & GP_PIXEL_PATTERN_FG_MASK) >> GP_PIXEL_PATTERN_FG_SHIFT;
}

/**
 * @brief Returns a pattern background for a grayscale pixel.
 *
 * @pixel A 1bpp pixel.
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
 * The table is then passed down to the gp_pixel_pattern_pixel() function to
 * determine a pixel value at a given coordinates.
 *
 * @param pattern A pixel pattern.
 * @return A byte array lookup table suitable.
 */
static inline uint8_t *gp_pixel_pattern_lookup_table_get(enum gp_pixel_pattern pattern)
{
	return gp_pixel_pattern_lookup_table[pattern];
}

/**
 * @brief Looks up a pixel color in a pattern based on x and y offsets.
 *
 * @param lookup A pattern lookup table.
 * @param fg A foreground pixel color.
 * @param bg A background pixel color.
 * @param x_mod An X offset of the pixel, i.e. x coordinate modulo GP_PIXEL_PATTERN_W.
 * @param y_mod An Y offset of the pixel, i.e. y coordinate modulo GP_PIXEL_PATTERN_H.
 *
 * @return A pixel value for a given coordinates and pattern.
 */
static inline gp_pixel gp_pixel_pattern_pixel_get(uint8_t *lookup, gp_pixel fg, gp_pixel bg,
                                                  uint8_t x_mod, uint8_t y_mod)
{
	return lookup[y_mod] & (8>>x_mod) ? fg : bg;
}

#endif /* CORE_GP_PATTERN_FILL_H */
