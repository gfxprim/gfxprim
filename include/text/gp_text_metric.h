// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_text_metric.h
 * @brief A text metric.
 *
 * @image html images/fonts/glyph_metrics.png
 */
#ifndef TEXT_GP_TEXT_METRIC_H
#define TEXT_GP_TEXT_METRIC_H

#include <core/gp_types.h>
#include <text/gp_text_style.h>

enum gp_text_len_type {
	/*
	 * Return bounding box, i.e. for the last letter return the greater of
	 * advance and letter width. This makes sure that the resulting size
	 * will fit the text.
	 */
	GP_TEXT_LEN_BBOX,
	/*
	 * Returns advance, i.e. where next letter would start after a string
	 * would have been printed. This may return slightly less than the
	 * GP_TEXT_LEN_BBOX.
	 */
	GP_TEXT_LEN_ADVANCE,
};

/**
 * @brief Returns a letter advance in horizontal direction.
 *
 * @param style A text style + font formatting
 * @param ch An unicode glyph.
 *
 * @return Horizontal advance in pixels.
 */
gp_size gp_glyph_advance_x(const gp_text_style *style, uint32_t ch);

/**
 * @brief Returns horizontal bearing.
 *
 * Bearing is a distance to be added to the x coordinate before we start
 * drawing a glyph bitmap. Bearing may be negative in some cases, typically for
 * letter 'J' and 'j'.
 *
 * @param style A text style + font formatting
 * @param ch An unicode glyph.
 *
 * @return Horizontal bearing in pixels.
 */
gp_ssize gp_glyph_bearing_x(const gp_text_style *style, uint32_t ch);

/**
 * @brief Calculates the width of the string drawn in the given style.
 *
 * @param style A text style + font formatting.
 * @param type Select if we want a bounding box or advance.
 * @param str A string.
 * @param len Maximal lenght of the string.
 *
 * @return Width in pixels.
 */
gp_size gp_text_width_len(const gp_text_style *style, enum gp_text_len_type type,
                          const char *str, size_t len);

/**
 * @brief Calculates bounding box width of the string drawn in the given style.
 *
 * @param style A text style + font formatting.
 * @param str A string.
 * @param len Maximal lenght of the string.
 *
 * @return Width in pixels.
 */
static inline gp_size gp_text_wbbox_len(const gp_text_style *style, const char *str, size_t len)
{
	return gp_text_width_len(style, GP_TEXT_LEN_BBOX, str, len);
}

/**
 * @brief Calculates the width of the string drawn in the given style.
 *
 * @param style A text style + font formatting.
 * @param type Select if we want a bounding box or advance.
 * @param str A string.
 *
 * @return Width in pixels.
 */
gp_size gp_text_width(const gp_text_style *style, enum gp_text_len_type type, const char *str);

/**
 * @brief Calculates bounding box width of the string drawn in the given style.
 *
 * @param style A text style + font formatting.
 * @param str A string.
 *
 * @return Width in pixels.
 */
static inline gp_size gp_text_wbbox(const gp_text_style *style, const char *str)
{
	return gp_text_width(style, GP_TEXT_LEN_BBOX, str);
}

/**
 * @brief Counts maximal width for len characters.
 *
 * @param style A text style + font formatting.
 * @param len A number of glyphs to draw.
 *
 * @return Width in pixels.
 */
gp_size gp_text_max_width(const gp_text_style *style, unsigned int len);

/**
 * @brief Counts average width for len characters.
 *
 * @param style A text style + font formatting.
 * @param len A number of glyphs to draw.
 *
 * @return Width in pixels.
 */
gp_size gp_text_avg_width(const gp_text_style *style, unsigned int len);

/**
 * @brief Returns maximal width for text written with len characters from str.
 *
 * First we find maximal width of an character from the given string, then we
 * use that to compute maximal width. Note that first and last character needs
 * special handling due to bearing and advance.
 *
 * @param style A text style + font formatting.
 * @param chars An UTF8 string.
 * @param len A number of glyphs to draw.
 *
 * @return Width in pixels.
 */
gp_size gp_text_max_width_chars(const gp_text_style *style, const char *chars,
                                unsigned int len);

/**
 * @brief Returns maximal text height.
 *
 * @param style A text style + font formatting.
 *
 * @return Height in pixels.
 */
gp_size gp_text_height(const gp_text_style *style);

/**
 * @brief Returns an ascent.
 *
 * Returns an ascent, the height from the baseline to the top of characters,
 * for the given text style.
 *
 * @param style A text style + font formatting.
 *
 * @return An ascent in pixels.
 */
gp_size gp_text_ascent(const gp_text_style *style);

/**
 * @brief Returns a descent.
 *
 * Returns a descend, the height from the baseline to the bottom of characters,
 * for the given text style.
 *
 * @param style A text style + font formatting.
 *
 * @return A descent in pixels.
 */
gp_size gp_text_descent(const gp_text_style *style);

/**
 * @brief Counts how many characters will fit into a width pixels.
 *
 * @param style A text style.
 * @param str An UTF-8 string.
 * @param width Width in pixels.
 *
 * @return How many characters from the string will fit the space.
 */
size_t gp_text_fit_width(const gp_text_style *style, const char *str,
                         gp_size width);

/**
 * @brief Returns position between string characters given an x pixel coordinate
 *        from the start of the rendered string. The return value is between
 *        [0, strlen(str)], where 0 means cursor before string and strlen(str)
 *        cursor after string.
 *
 * @param style A text style.
 * @param str An UTF8 string.
 * @param x_off Horizontal offset in the rendered string in pixel.
 *
 * @return A cursor position in the string.
 */
gp_size gp_text_cur_pos(const gp_text_style *style, const char *str, gp_coord x_off);

#endif /* TEXT_GP_TEXT_METRIC_H */
