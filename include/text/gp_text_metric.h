// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos
 *                         <jiri.bluebear.dluhos@gmail.com>
 *
 * Copyright (C) 2009-2021 Cyril Hrubis <metan@ucw.cz>
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
 * @brief Calculates the width of the string drawn in the given style.
 *
 * @style A text style + font formatting.
 * @type Select if we want a bounding box or advance.
 * @str A string.
 * @len Maximal lenght of the string.
 *
 * @return Width in pixels.
 */
gp_size gp_text_width_len(const gp_text_style *style, enum gp_text_len_type type,
                          const char *str, size_t len);

/**
 * @brief Calculates bounding box width of the string drawn in the given style.
 *
 * @style A text style + font formatting.
 * @str A string.
 * @len Maximal lenght of the string.
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
 * @style A text style + font formatting.
 * @type Select if we want a bounding box or advance.
 * @str A string.
 *
 * @return Width in pixels.
 */
gp_size gp_text_width(const gp_text_style *style, enum gp_text_len_type type, const char *str);

/**
 * @brief Calculates bounding box width of the string drawn in the given style.
 *
 * @style A text style + font formatting.
 * @str A string.
 *
 * @return Width in pixels.
 */
static inline gp_size gp_text_wbbox(const gp_text_style *style, const char *str)
{
	return gp_text_width(style, GP_TEXT_LEN_BBOX, str);
}

/*
 * Maximal text width for string with len characters.
 */
gp_size gp_text_max_width(const gp_text_style *style, unsigned int len);

/*
 * Returns average width for len characters.
 */
gp_size gp_text_avg_width(const gp_text_style *style, unsigned int len);

/*
 * Returns maximal width for text written with len characters from str.
 */
gp_size gp_text_max_width_chars(const gp_text_style *style, const char *chars,
                                unsigned int len);

/*
 * Returns maximal text height, in pixels.
 */
gp_size gp_text_height(const gp_text_style *style);

/*
 * Returns the ascent (height from the baseline to the top of characters),
 * for the given text style. (Result is in pixels.)
 */
gp_size gp_text_ascent(const gp_text_style *style);

/*
 * Returns the descent (height from the baseline to the bottom of characters),
 * for the given text style. (Result is in pixels.)
 */
gp_size gp_text_descent(const gp_text_style *style);

/**
 * @brief Returns how many characters will fit into a width pixels.
 *
 * @style A text style.
 * @chars A string.
 * @width Width in pixels.
 */
size_t gp_text_fit_width(const gp_text_style *style, const char *str,
                         gp_size width);

/**
 * @brief Returns position between string characters given an x pixel coordinate
 *        from the start of the rendered string. The return value is between
 *        [0, strlen(str)], where 0 means cursor before string and strlen(str)
 *        cursor after string.
 *
 * @style A text style + font formatting.
 * @str A string.
 * @x_off Horizontal offset in the rendered string in pixel.
 *
 * @return A cursor position in the string.
 */
gp_size gp_text_cur_pos(const gp_text_style *style, const char *str, gp_coord x_off);

#endif /* TEXT_GP_TEXT_METRIC_H */
