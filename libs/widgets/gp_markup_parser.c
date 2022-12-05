//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <core/gp_debug.h>
#include <widgets/gp_markup_parser.h>

static char *strcopy(char **buf, const char *str, size_t len)
{
	char *ret = *buf;

	strncpy(ret, str, len);
	ret[len] = 0;

	*buf += len + 1;

	return ret;
}

static int parse_markup_var(const char *markup, unsigned int type,
                            unsigned int attrs, gp_markup_elem **elems, char **buf)
{
	unsigned int i = 1;

	while (markup[i] && markup[i] != '}')
		i++;

	if (!markup[i]) {
		GP_WARN("Unfinished markup variable!");
		return -1;
	}

	if (*elems) {
		(*elems)->type = type;
		(*elems)->str = strcopy(buf, markup + 1, i - 1);
		(*elems)->attrs = attrs;
		(*elems)++;
	}

	return i + 1;
}

static int parse_markup_string(const char *markup, unsigned int len, unsigned int attrs, gp_markup_elem **elems, char **buf)
{
	if (!len)
		return 0;

	if (!(*elems))
		return 1;

	(*elems)->type = GP_MARKUP_STR;
	(*elems)->str = strcopy(buf, markup, len);
	(*elems)->attrs = attrs;
	(*elems)++;

	return 1;
}

static void markup_newline(gp_markup_elem **elems)
{
	if (!(*elems))
		return;

	(*elems)->type = GP_MARKUP_NEWLINE;
	(*elems)++;
}

static int parse_markup(const char *markup, gp_markup_elem *elems, char *buf)
{
	unsigned int i;
	unsigned int j = 0;
	int ret = 0;
	int r;
	char prev_ch = 0;
	int attrs = 0;
	unsigned int type;

	for (i = 0; markup[i]; i++) {
		switch (markup[i]) {
		case '{':
			if (prev_ch == '\\') {
				ret += parse_markup_string(&markup[j], i - j - 1, attrs, &elems, &buf);
				j = i;
				continue;
			}

			ret += parse_markup_string(&markup[j], i - j, attrs, &elems, &buf);

			type = GP_MARKUP_VAR;

			if (prev_ch == '_' || prev_ch == '^')
				type = GP_MARKUP_STR;

			r = parse_markup_var(&markup[i], type, attrs, &elems, &buf);
			if (r < 0)
				return -1;

			if (prev_ch == '_' || prev_ch == '^')
				attrs &= ~(GP_MARKUP_SUBSCRIPT | GP_MARKUP_SUPERSCRIPT);

			i += r;
			j = i;
			ret++;
		break;
		case '*':
			if (prev_ch == '\\') {
				ret += parse_markup_string(&markup[j], i - j - 1, attrs, &elems, &buf);
				j = i;
				continue;
			}

			ret += parse_markup_string(&markup[j], i - j, attrs, &elems, &buf);
			attrs ^= GP_MARKUP_BOLD;
			j = i + 1;
		break;
		case '#':
			if (prev_ch == '\\') {
				ret += parse_markup_string(&markup[j], i - j - 1, attrs, &elems, &buf);
				j = i;
				continue;
			}

			ret += parse_markup_string(&markup[j], i - j, attrs, &elems, &buf);
			attrs ^= GP_MARKUP_BIG;
			j = i + 1;
		break;
		case '`':
			if (prev_ch == '\\') {
				ret += parse_markup_string(&markup[j], i - j - 1, attrs, &elems, &buf);
				j = i;
				continue;
			}

			ret += parse_markup_string(&markup[j], i - j, attrs, &elems, &buf);
			attrs ^= GP_MARKUP_INVERSE;
			j = i + 1;
		break;
		case '_':
			if (prev_ch == '\\') {
				ret += parse_markup_string(&markup[j], i - j - 1, attrs, &elems, &buf);
				j = i;
				continue;
			}

			ret += parse_markup_string(&markup[j], i - j, attrs, &elems, &buf);
			attrs &= ~GP_MARKUP_SUPERSCRIPT;
			attrs |= GP_MARKUP_SUBSCRIPT;
			j = i + 1;
		break;
		case '^':
			if (prev_ch == '\\') {
				ret += parse_markup_string(&markup[j], i - j - 1, attrs, &elems, &buf);
				j = i;
				continue;
			}

			ret += parse_markup_string(&markup[j], i - j, attrs, &elems, &buf);
			attrs &= ~GP_MARKUP_SUBSCRIPT;
			attrs |= GP_MARKUP_SUPERSCRIPT;
			j = i + 1;
		break;
		case ' ':
			ret += parse_markup_string(&markup[j], i - j, attrs, &elems, &buf);
			j = i;
			attrs &= ~(GP_MARKUP_SUBSCRIPT | GP_MARKUP_SUPERSCRIPT);
		break;
		case '\n':
			ret += parse_markup_string(&markup[j], i - j, attrs, &elems, &buf);
			markup_newline(&elems);
			j = i + 1;
			ret++;
			attrs &= ~(GP_MARKUP_SUBSCRIPT | GP_MARKUP_SUPERSCRIPT);
		break;
		}

		prev_ch = markup[i];
	}

	ret += parse_markup_string(&markup[j], i - j, attrs, &elems, &buf);

	return ret;
}

void gp_markup_dump(gp_markup *self)
{
	gp_markup_elem *e;

	for (e = gp_markup_first(self); e->type != GP_MARKUP_END; e++) {
		switch (e->type) {
		case GP_MARKUP_STR:
			printf("STR: '%s' attrs %i\n", e->str, e->attrs);
		break;
		case GP_MARKUP_VAR:
			printf("VAR: '%s' attrs %i\n", gp_markup_elem_str(e), e->attrs);
		break;
		case GP_MARKUP_NEWLINE:
			printf("NL\n");
		break;
		}
	}
}

gp_markup *gp_markup_gfxprim_parse(const char *markup)
{
	int elem_cnt = parse_markup(markup, NULL, NULL);
	struct gp_markup *ret;
	char *buf;

	if (elem_cnt < 0)
		return NULL;

	ret = calloc(sizeof(struct gp_markup) + sizeof(struct gp_markup_elem) * (elem_cnt + 1), 1);
	if (!ret)
		return NULL;

	buf = malloc(strlen(markup) + elem_cnt);
	if (!buf) {
		free(ret);
		return NULL;
	}

	parse_markup(markup, ret->elems, buf);

	ret->elems[elem_cnt].type = GP_MARKUP_END;

	return ret;
}

gp_markup *gp_markup_parse(enum gp_markup_fmt fmt, const char *markup_str)
{
	switch (fmt) {
	case GP_MARKUP_GFXPRIM:
		return gp_markup_gfxprim_parse(markup_str);
	case GP_MARKUP_HTML:
		return gp_markup_html_parse(markup_str);
	}

	GP_BUG("Invalid markup format %i\n", fmt);
	return NULL;
}

void gp_markup_free(gp_markup *self)
{
	free(self->markup);
	free(self);
}
