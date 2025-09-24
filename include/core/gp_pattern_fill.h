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
 * @attention Most pattern fills work only with fill gfx functions.
 */
enum gp_pixel_patterns {
	/**
	 * @brief No pattern.
	 *
	 * If uppter bits of the byte are zeroed it's a solid color.
	 */
	GP_PIXEL_PATTERN_NONE = 0x0000,

	/**
	 * This patterns pattern draws every other pixel and swaps offset on each line:
	 *
	 * @code
	 * # # # # # #
	 *  # # # # #
	 * # # # # # #
	 *  # # # # #
	 * # # # # # #
	 * @endcode
	 *
	 *
	 * This pattern value can be also passed to all hline and vline
	 * functions where it will draw a dotted line that matches the
	 * placement of white and black pixels in the pattern.
	 */
	GP_PIXEL_PATTERN_50 = 0x0100,
};

/**
 * @brief Pattern mask.
 *
 * After applying a pattern mask on a pixel the value can be compared
 * to the pattern constants.
 */
#define GP_PIXEL_PATTERN_MASK 0xff00

/**
 * @brief Returns a pattern for a grayscale pixel.
 *
 * @pixel A 1bpp pixel.
 * @return A pattern constant.
 */
static inline enum gp_pixel_patterns gp_pixel_pattern(gp_pixel pixel)
{
	return pixel & GP_PIXEL_PATTERN_MASK;
}

#endif /* CORE_GP_PATTERN_FILL_H */
