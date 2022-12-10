//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2023 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <core/gp_debug.h>
#include <utils/gp_utf.h>
#include <utils/gp_markup_parser.h>
#include <utils/gp_markup_builder.h>

static void parse_markup(const char *markup, gp_markup_builder *builder)
{
	uint32_t glyph, prev_glyph = 0;

	while ((glyph = gp_utf8_next(&markup))) {
		switch (glyph) {
		case '\r':
		case '\n':
			if ((prev_glyph == '\r' && glyph == '\n') ||
			    (prev_glyph == '\n' && glyph == '\r')) {
				prev_glyph = 0;
				continue;
			}

			gp_markup_builder_newline(builder);
		break;
		case '\t':
		case ' ':
			gp_markup_builder_space(builder, 0);
		break;
		default:
			if (glyph > 0x20)
				gp_markup_builder_glyph(builder, glyph, 0, 0);
		}

		prev_glyph = glyph;
	}
}

gp_markup *gp_markup_plaintext_parse(const char *markup, enum gp_markup_flags flags)
{
	struct gp_markup_builder builder = {};
	struct gp_markup *ret;

	if (flags) {
		GP_WARN("Invalid flags");
		return NULL;
	}

	parse_markup(markup, &builder);

	ret = gp_markup_builder_alloc(&builder);
	if (!ret)
		return NULL;

	parse_markup(markup, &builder);

	gp_markup_builder_finish(&builder);

	return ret;
}
