// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef UTILS_GP_MARKUP_PARSER_H
#define UTILS_GP_MARKUP_PARSER_H

#include <utils/gp_markup.h>

enum gp_markup_fmt {
	GP_MARKUP_PLAINTEXT,
	GP_MARKUP_GFXPRIM,
	GP_MARKUP_HTML,
};

/**
 * @brief Parsers a markup string.
 *
 * @fmt A markup format.
 * @markup A string with a markup.
 * @flags A flags passed to markup parser.
 *
 * @return A parsed markup or NULL in a case of a failure.
 */
gp_markup *gp_markup_parse(enum gp_markup_fmt fmt, const char *markup,
                           enum gp_markup_flags flags);

/**
 * @brief Parses a plaintext markup.
 *
 * @markup A string with a markup.
 * @flags A flags passed to markup parser.
 *
 * @return A parsed markup or NULL in a case of a failure.
 */
gp_markup *gp_markup_plaintext_parse(const char *markup,
                                     enum gp_markup_flags flags);

/**
 * @brief Parses a gfxprim markup.
 *
 * @markup A string with a markup.
 * @flags A flags passed to markup parser.
 *
 * @return A parsed markup or NULL in a case of a failure.
 */
gp_markup *gp_markup_gfxprim_parse(const char *markup,
                                   enum gp_markup_flags flags);

/**
 * @brief Parses a html markup.
 *
 * @markup A string with a markup.
 * @flags A flags passed to markup parser.
 *
 * @return A parsed markup or NULL in a case of a failure.
 */
gp_markup *gp_markup_html_parse(const char *markup,
                                enum gp_markup_flags flags);

/**
 * @brief Frees a markup.
 *
 * @markup A parsed markup.
 */
void gp_markup_free(gp_markup *self);

#endif /* UTILS_GP_MARKUP_PARSER_H */
