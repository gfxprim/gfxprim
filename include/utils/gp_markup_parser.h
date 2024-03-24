// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_markup_parser.h
 * @brief A markup parsers.
 */

#ifndef UTILS_GP_MARKUP_PARSER_H
#define UTILS_GP_MARKUP_PARSER_H

#include <utils/gp_markup.h>

/**
 * @brief Markup format to parse.
 */
enum gp_markup_fmt {
	/** An UTF-8 text without any formatting */
	GP_MARKUP_PLAINTEXT,
	/** A GFXprim markup */
	GP_MARKUP_GFXPRIM,
	/** A small subset of HTML */
	GP_MARKUP_HTML,
};

/**
 * @brief Parsers a markup string.
 *
 * @param fmt A markup format.
 * @param markup A string with a markup.
 * @param flags A flags passed to markup parser.
 *
 * @return A parsed markup or NULL in a case of a failure.
 */
gp_markup *gp_markup_parse(enum gp_markup_fmt fmt, const char *markup,
                           enum gp_markup_flags flags);

/**
 * @brief Parses a plaintext markup.
 *
 * @param markup A string with a markup.
 * @param flags Flags passed to markup parser.
 *
 * @return A parsed markup or NULL in a case of a failure.
 */
gp_markup *gp_markup_plaintext_parse(const char *markup,
                                     enum gp_markup_flags flags);

/**
 * @brief Parses a gfxprim markup.
 *
 * @param markup A string with a markup.
 * @param flags Flags passed to markup parser.
 *
 * @return A parsed markup or NULL in a case of a failure.
 */
gp_markup *gp_markup_gfxprim_parse(const char *markup,
                                   enum gp_markup_flags flags);

/**
 * HTML parser markup flags.
 */
enum gp_markup_html_flags {
	/**
	 * Keep whitespaces
	 *
	 * Some HTML like markup languages only use tags for text size/color
	 * and expect whitespaces to be kept, such as Pango markup or XDXF.
	 */
	GP_MARKUP_HTML_KEEP_WS = 0x01,
};

/**
 * @brief Parses a HTML markup.
 *
 * @param markup A string with a markup.
 * @param flags A flags passed to markup parser.
 *
 * @return A parsed markup or NULL in a case of a failure.
 */
gp_markup *gp_markup_html_parse(const char *markup,
                                enum gp_markup_flags flags);

/**
 * @brief Frees a markup.
 *
 * @param markup A parsed markup.
 */
void gp_markup_free(gp_markup *self);

#endif /* UTILS_GP_MARKUP_PARSER_H */
