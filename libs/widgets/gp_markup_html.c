//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <widgets/gp_markup_parser.h>

struct counters {
	unsigned int elems;
	size_t buffers;
};

struct html_esc {
	const char *name;
	const char *val;
} html_escs[] = {
	{"amp", "&"},
	{"lt", "<"},
	{"gt", ">"},
	{"nbsp", "\u00a0"},
	{"copy", "\u00a9"},
	{"reg", "\u00ae"},
	{"deg", "\u00b0"},
	{"plusmn", "\u00b1"},
	{"micro", "\u00b4"},
};

static const char *html_esc_lookup(const char *str, size_t len)
{
	size_t i;

	for (i = 0; i < GP_ARRAY_SIZE(html_escs); i++) {
		if (!strncmp(str, html_escs[i].name, len))
			return html_escs[i].val;
	}

	return NULL;
}

static char *strcopy(char **buf, const char *str, size_t len)
{
	char *ret = *buf, *tmp = *buf;
	const char *esc;
	size_t i = 0;
	char in_ws = 0;

	while (i < len) {
		switch (str[i]) {
		case '&':
			esc = &str[++i];

			while (str[i] && str[i] != ';')
				i++;

			if (str[i] != ';')
				continue;

			esc = html_esc_lookup(esc, str+i-esc);

			i++;

			if (esc) {
				strcpy(tmp, esc);
				tmp += strlen(esc);
				in_ws = 0;
			}
		break;
		case ' ':
		case '\t':
		case '\n':
			if (!in_ws)
				*(tmp++) = ' ';
			i++;
			in_ws = 1;
		break;
		default:
			*(tmp++)=str[i++];
			in_ws = 0;
		}
	}

	*tmp = 0;

	*buf += len + 1;

	return ret;
}

static void parse_markup_string(struct counters *counters,
                                const char *markup, unsigned int len, unsigned int attrs,
                                gp_markup_elem **elems, char **buf)
{
	if (!len)
		return;

	if (counters) {
		counters->elems++;
		counters->buffers += len + 1;
	}

	if (!(*elems))
		return;

	(*elems)->type = GP_MARKUP_STR;
	(*elems)->str = strcopy(buf, markup, len);
	(*elems)->attrs = attrs;
	(*elems)++;
}

static void markup_push(struct counters *counters, gp_markup_elem **elems,
                        enum gp_markup_elem_type type)
{
	if (counters)
		counters->elems++;

	if (!(*elems))
		return;

	(*elems)->type = type;
	(*elems)++;
}

enum html_tag {
	HTML_UNKNOWN,
	HTML_B,
	HTML_BR,
	HTML_BIG,
	HTML_HR,
	HTML_I,
	HTML_S,
	HTML_SMALL,
	HTML_SUB,
	HTML_SUP,
	HTML_TT,
	HTML_U,
};

static struct html_tags {
	const char *str;
	enum html_tag tag;
} tags[] = {
	{"b", HTML_B},
	{"B", HTML_B},
	{"br", HTML_BR},
	{"BR", HTML_BR},
	{"br /", HTML_BR},
	{"BR /", HTML_BR},
	{"big", HTML_BIG},
	{"BIG", HTML_BIG},
	{"hr", HTML_HR},
	{"hr /", HTML_HR},
	{"HR", HTML_HR},
	{"HR /", HTML_HR},
	//{"i", HTML_I},
	//{"s", HTML_S},
	//{"small", HTML_SMALL},
	{"sub", HTML_SUB},
	{"SUB", HTML_SUB},
	{"sup", HTML_SUP},
	{"SUP", HTML_SUP},
	//{"tt", HTML_TT},
	//{"u", HTML_U},
};

static enum html_tag parse_tag(const char *buf, size_t start, size_t end)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(tags); i++) {
		if (!strncmp(buf + start, tags[i].str, end - start))
			return tags[i].tag;
	}

	return HTML_UNKNOWN;
}

static inline void attr_bit(int *attr, int is_closing, int bit)
{
	if (is_closing)
		*attr &= ~bit;
	else
		*attr |= bit;
}

static size_t html_tag(struct counters *counters, const char *buf,
                       gp_markup_elem **elems, int *attr)
{
	size_t start = 0, end = 0;
	int is_closing = 0;
	enum html_tag html_tag;

	if (buf[end] == '/') {
		is_closing = 1;
		end++;
		start++;
	}

	while (buf[end] && buf[end] != '>')
		end++;

	if (!buf[end])
		return end;

	if (start == end)
		return 2;

	html_tag = parse_tag(buf, start, end);

	switch (html_tag) {
	case HTML_B:
		attr_bit(attr, is_closing, GP_MARKUP_BOLD);
	break;
	case HTML_BR:
		markup_push(counters, elems, GP_MARKUP_NEWLINE);
	break;
	case HTML_BIG:
		attr_bit(attr, is_closing, GP_MARKUP_BIG);
	break;
	case HTML_HR:
		markup_push(counters, elems, GP_MARKUP_HLINE);
	break;
	case HTML_SUP:
		attr_bit(attr, is_closing, GP_MARKUP_SUBSCRIPT);
	break;
	case HTML_SUB:
		attr_bit(attr, is_closing, GP_MARKUP_SUPERSCRIPT);
	break;
	}

	return end + 1;
};

static void parse_markup(struct counters *counters, const char *markup, gp_markup_elem *elems, char *buf)
{
	int attr = 0;
	const char *str_start = markup;

	while (*markup) {
		if (*markup == '<') {
			markup++;
			markup += html_tag(counters, markup, &elems, &attr);
			str_start = markup;
			continue;
		}

		while (*markup && *markup != '<')
			markup++;

		parse_markup_string(counters, str_start, markup-str_start, attr, &elems, &buf);
	}
}

gp_markup *gp_markup_html_parse(const char *markup)
{
	struct counters counters = {};
	struct gp_markup *ret;
	char *buf;

	parse_markup(&counters, markup, NULL, NULL);

	ret = calloc(sizeof(struct gp_markup) + sizeof(struct gp_markup_elem) * (counters.elems + 1), 1);
	if (!ret)
		return NULL;

	buf = malloc(counters.buffers);
	if (!buf) {
		free(ret);
		return NULL;
	}

	parse_markup(NULL, markup, ret->elems, buf);

	ret->elems[counters.elems].type = GP_MARKUP_END;

	return ret;
}
