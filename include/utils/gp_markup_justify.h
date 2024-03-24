// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_markup_justify.h
 * @brief Justifies markup into lines.
 */

#ifndef UTILS_GP_MARKUP_JUSTIFY_H
#define UTILS_GP_MARKUP_JUSTIFY_H

#include <utils/gp_markup.h>

/**
 * @brief A markup line.
 */
typedef struct gp_markup_line {
	/** Pointer to a first glyph in the line. */
	gp_markup_glyph *first;
	/** Pointer to last glyph in the line. */
	gp_markup_glyph *last;
} gp_markup_line;

/**
 * @brief A markup lines.
 *
 * This represents a markup text justified into a lines.
 */
typedef struct gp_markup_lines {
	/** Number of justified text lines. */
	unsigned int lines_cnt;
	/** Array of the lines. */
	gp_markup_line lines[];
} gp_markup_lines;

/**
 * @brief A callback for text justification.
 *
 * The algorithm that breaks text into lines needs to know the width of a group
 * of glyphs in order to be able to break the text into lines of a given width.
 *
 * @param first A pointer to a start of the array of glyphs.
 * @param len A number of glyphs in the array.
 * @param priv A private pointer passed to the gp_markup_justify() function.
 *
 * @return A width of the glyphs.
 */
typedef unsigned int (*gp_markup_width_cb)(gp_markup_glyph *first, size_t len, void *priv);


/**
 * @brief Justifies text into lines.
 *
 * @param self A text markup.
 * @param line_width A width the markup should be justified into.
 * @param width_callback A callback that computes width of a glyph group.
 * @param priv A private pointer passed down to the width_callback.
 *
 */
gp_markup_lines *gp_markup_justify(gp_markup *self, unsigned int line_width, gp_markup_width_cb width_callback, void *priv);

/**
 * @brief Frees justified lines.
 *
 * @param self A justified markup.
 */
void gp_markup_justify_free(gp_markup_lines *self);

/**
 * @brief Dumps justified lines into stdout, used for debugging.
 *
 * @param self A justified markup.
 */
void gp_markup_justify_dump(gp_markup_lines *self);

#endif /* UTILS_GP_MARKUP_JUSTIFY_H */
