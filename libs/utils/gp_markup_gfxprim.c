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
	int attrs = 0, in_esc = 0;
	uint32_t glyph;

	while ((glyph = gp_utf8_next(&markup))) {
		switch (glyph) {
		case '{':
			if (in_esc) {
				gp_markup_builder_glyph(builder, glyph, attrs, 0);
				continue;
			}

			while (*markup && *markup != '}')
				markup++;
		break;
		case '*':
			if (in_esc)
				gp_markup_builder_glyph(builder, glyph, attrs, 0);
			else
				attrs ^= GP_MARKUP_BOLD;
		break;
		case '#':
			if (in_esc)
				gp_markup_builder_glyph(builder, glyph, attrs, 0);
			else
				attrs ^= GP_MARKUP_LARGE;
		break;
		case '_':
			if (in_esc) {
				gp_markup_builder_glyph(builder, glyph, attrs, 0);
			} else {
				attrs &= ~GP_MARKUP_SUP;
				attrs ^= GP_MARKUP_SUB;
			}
		break;
		case '^':
			if (in_esc) {
				gp_markup_builder_glyph(builder, glyph, attrs, 0);
			} else {
				attrs &= ~GP_MARKUP_SUB;
				attrs ^= GP_MARKUP_SUP;
			}
		break;
		case '~':
			if (in_esc)
				gp_markup_builder_glyph(builder, glyph, attrs, 0);
			else
				attrs ^= GP_MARKUP_UNDERLINE;
		break;
		case '`':
			if (in_esc)
				gp_markup_builder_glyph(builder, glyph, attrs, 0);
			else
				attrs ^= GP_MARKUP_MONO;
		break;
		case '\t':
		case ' ':
			gp_markup_builder_space(builder, attrs);
		break;
		case '\n':
			gp_markup_builder_newline(builder);
			attrs &= ~(GP_MARKUP_SUB | GP_MARKUP_SUP);
		break;
		case '\\':
			if (in_esc) {
				gp_markup_builder_glyph(builder, glyph, attrs, 0);
				in_esc = 0;
			} else {
				in_esc = 1;
			}
		break;
		default:
			if (glyph > 0x20)
				gp_markup_builder_glyph(builder, glyph, attrs, 0);
		}

		if (in_esc && glyph != '\\')
			in_esc = 0;
	}
}

gp_markup *gp_markup_gfxprim_parse(const char *markup, enum gp_markup_flags flags)
{
	gp_markup_builder builder = {};
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
