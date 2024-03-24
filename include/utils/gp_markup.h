// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_markup.h
 * @brief A simple text markup format.
 */

#ifndef UTILS_GP_MARKUP_H
#define UTILS_GP_MARKUP_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Markup format flags.
 *
 * These bitflags can be combined bitwise.
 */
enum gp_markup_flags {
	/** Bold text */
	GP_MARKUP_BOLD = 0x01,
	/** Large text */
	GP_MARKUP_LARGE = 0x02,
	/** Monospace text */
	GP_MARKUP_MONO = 0x04,
	/** Subscript text */
	GP_MARKUP_SUB = 0x08,
	/** Superscript text */
	GP_MARKUP_SUP = 0x10,
	/** Underlined text */
	GP_MARKUP_UNDERLINE = 0x20,
	/** Striked text */
	GP_MARKUP_STRIKE = 0x40,
};

/**
 * @brief A markup color.
 *
 * Default color + 16 colors.
 */
enum gp_markup_color {
	/** Default color */
	GP_MARKUP_DEFAULT = 0x00,
	/* 16 colors */
	GP_MARKUP_RED,
	GP_MARKUP_GREEN,
	GP_MARKUP_YELLOW,
	GP_MARKUP_BLUE,
	GP_MARKUP_MAGENTA,
	GP_MARKUP_CYAN,
	GP_MARKUP_GRAY,
	GP_MARKUP_BR_RED,
	GP_MARKUP_BR_GREEN,
	GP_MARKUP_BR_YELLOW,
	GP_MARKUP_BR_BLUE,
	GP_MARKUP_BR_MAGENTA,
	GP_MARKUP_BR_CYAN,
	GP_MARKUP_WHITE,
};

/**
 * @brief A markup glyph.
 */
typedef struct gp_markup_glyph {
	/** @brief An unicode glyph */
	uint32_t glyph;
	/**
	 * @brief A text format.
	 *
	 * A bitwise combinations of enum gp_markup_flags.
	 */
	uint16_t fmt;
	union {
		/**
		 * @brief A text color.
		 *
		 * The enum gp_markup_color.
		 */
		uint16_t fg_color;
		int16_t space_padd;
	};
} gp_markup_glyph;

/**
 * @brief A text markup.
 *
 * This represents a text with a markup.
 *
 * Markups can be parsed from different formats with gp_markup_parse().
 *
 * To display the text the markup needs to be dividies into lines, see
 * gp_markup_justify().
 */
typedef struct gp_markup {
	/**
	 * @brief Number of glyphs.
	 *
	 * The size of the glyphs array.
	 */
	uint32_t glyph_cnt;
	/**
	 * @brief An array of glyphs.
	 *
	 * The array is null terminated, i.e. the last glyph value is always 0.
	 */
	gp_markup_glyph glyphs[];
} gp_markup;

/**
 * @brief Dumps markup into the stdout, this is just for debugging.
 *
 * @param self A markup.
 */
void gp_markup_dump(gp_markup *self);

#endif /* UTILS_GP_MARKUP_H */
