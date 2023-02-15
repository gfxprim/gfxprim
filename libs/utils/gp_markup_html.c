//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

/*

   This is stupid linear HTML parser. It does support only basic tags and we do
   not care if the HTML is even close to valid or not.

   It does support a few basic tags such as <br> <b> but only one level of
   <sub> and <sup> is supported.

   The level of nested <span> tags is limited as well.

 */

#include <string.h>

#include <core/gp_debug.h>
#include <core/gp_common.h>
#include <utils/gp_utf.h>
#include <utils/gp_markup_parser.h>
#include <utils/gp_markup_builder.h>

struct sub_str {
	const char *first;
	const char *last;
};

static inline int sub_str_nempty(const struct sub_str *sub_str)
{
	return sub_str->first <= sub_str->last;
}

static inline int sub_str_empty(const struct sub_str *sub_str)
{
	return sub_str->first > sub_str->last;
}

/* stack for text attributes in nested tags */

#define ATTR_STACK_MAX 128

struct attr_stack {
	size_t cur_level;
	size_t stack_top;
	int16_t stack[ATTR_STACK_MAX];
	int16_t color[ATTR_STACK_MAX];
};

static inline void attr_stack_push(struct attr_stack *self, int attrs_on, int attrs_off, int color)
{
	self->cur_level++;

	if (self->cur_level >= GP_ARRAY_SIZE(self->stack))
		return;

	self->stack_top++;

	self->stack[self->stack_top] = self->stack[self->stack_top-1];

	self->stack[self->stack_top] &= ~attrs_off;
	self->stack[self->stack_top] |= attrs_on;

	if (color > 0)
		self->color[self->stack_top] = color;
}

static inline void attr_stack_pop(struct attr_stack *self)
{
	if (self->cur_level >= GP_ARRAY_SIZE(self->stack)) {
		self->cur_level--;
		return;
	}

	if (!self->stack_top)
		return;

	self->cur_level--;
	self->stack_top--;
}

static inline int attr_stack_fmt(struct attr_stack *self)
{
	return self->stack[self->stack_top];
}

static inline uint16_t attr_stack_color(struct attr_stack *self)
{
	return self->color[self->stack_top];
}

static inline void attr_stack_on(struct attr_stack *self, int attr)
{
	if (self->cur_level >= GP_ARRAY_SIZE(self->stack))
		return;

	self->stack[self->stack_top] |= attr;
}

static inline void attr_stack_off(struct attr_stack *self, int attr)
{
	if (self->cur_level >= GP_ARRAY_SIZE(self->stack))
		return;

	self->stack[self->stack_top] &= ~attr;
}

/* HTML whitespaces */
static inline int is_space(char ch)
{
	switch (ch) {
	case ' ':
	case '\n':
	case '\t':
	case '\r':
		return 1;
	default:
		return 0;
	}
}

static void html_eat_ws(struct sub_str *substr)
{
	while (sub_str_nempty(substr)) {
		if (!is_space(*substr->first))
			return;

		substr->first++;
	}
}

static struct html_esc {
	const char *name;
	uint32_t val;
} html_escs[] = {
	{"amp", '&'},
	{"lt", '<'},
	{"gt", '>'},
	{"nbsp", 0xa0},
	{"copy", 0xa9},
	{"reg", 0xae},
	{"deg", 0xb0},
	{"plusmn", 0xb1},
	{"micro", 0xb4},
};

static uint32_t html_esc_lookup(const char *str, size_t len)
{
	size_t i;

	for (i = 0; i < GP_ARRAY_SIZE(html_escs); i++) {
		if (!strncmp(str, html_escs[i].name, len))
			return html_escs[i].val;
	}

	return 0;
}

static void parse_markup_string(gp_markup_builder *builder,
                                const char *first, const char *end,
                                struct attr_stack *attrs)
{
	while (first < end) {
		uint32_t glyph = gp_utf8_next(&first);

		if (glyph == '&') {
			const char *esc = first;

			while (first < end && *first != ';')
				first++;

			if (*first != ';')
				return;

			glyph = html_esc_lookup(esc, first-esc);

			first++;

			if (!glyph)
				continue;
		}

		gp_markup_builder_glyph(builder, glyph, attr_stack_fmt(attrs), attr_stack_color(attrs));
	}
}

enum html_tag {
	HTML_UNKNOWN,
	HTML_B,
	HTML_BR,
	HTML_BIG,
	HTML_HR,
	HTML_P,
//	HTML_I,
	HTML_S,
	HTML_SCRIPT,
//	HTML_SMALL,
	HTML_SPAN,
	HTML_STYLE,
	HTML_SUB,
	HTML_SUP,
	HTML_TITLE,
	HTML_TT,
	HTML_U,
	HTML_XML,
};

static struct html_tags {
	const char *str;
	enum html_tag tag;
	size_t cmp_len;
} tags[] = {
	{"b", HTML_B, 0},
	{"br", HTML_BR, 0},
	{"br /", HTML_BR, 0},
	{"big", HTML_BIG, 0},
	{"hr", HTML_HR, 0},
	{"hr /", HTML_HR, 0},
	//{"i", HTML_I},
	{"p", HTML_P, 0},
	{"s", HTML_S, 0},
	//{"small", HTML_SMALL},
	{"script", HTML_SCRIPT, 6},
	{"span", HTML_SPAN, 4},
	{"style", HTML_STYLE, 0},
	{"sub", HTML_SUB, 0},
	{"sup", HTML_SUP, 0},
	{"title", HTML_TITLE, 0},
	{"tt", HTML_TT, 0},
	{"u", HTML_U, 0},
	{"xml", HTML_XML, 0},
};

struct html_tag_res {
	enum html_tag tag;
	struct sub_str attrs;
};

static struct html_tag_res parse_tag(const char *first, const char *last, int is_closing)
{
	unsigned int i;

	for (i = 0; i < GP_ARRAY_SIZE(tags); i++) {
		size_t cmp_len = (tags[i].cmp_len && !is_closing) ? tags[i].cmp_len : (size_t)(last - first);

		if (!strncasecmp(first, tags[i].str, cmp_len)) {
			struct html_tag_res res = {
				.tag = tags[i].tag,
				.attrs = {.first = first + cmp_len, .last = last-1}
			};
			return res;
		}
	}

	return (struct html_tag_res){.tag = HTML_UNKNOWN};
}

static inline void attr_bit(struct attr_stack *attrs, int is_closing, int bit)
{
	if (is_closing)
		attr_stack_off(attrs, bit);
	else
		attr_stack_on(attrs, bit);
}

static void strip_quotes(const char **buf, size_t *len)
{
	if (*len < 2)
		return;

	if ((*buf)[0] == '"' && (*buf)[*len-1] == '"') {
		(*buf)++;
		*len-=2;
	}
}

static int parse_font_weight(const char *buf, size_t len,
                             int *attrs_on, int *attrs_off, int *color)
{
	(void) color;

	strip_quotes(&buf, &len);

	if (!strncasecmp(buf, "bold", len) ||
	    !strncasecmp(buf, "ultrabold", len) ||
	    !strncasecmp(buf, "heavy", len)) {
		*attrs_on |= GP_MARKUP_BOLD;
		return 1;
	}

	if (!strncasecmp(buf, "ultralight", len) ||
	    !strncasecmp(buf, "light", len) ||
	    !strncasecmp(buf, "normal", len)) {
		*attrs_off |= GP_MARKUP_BOLD;
		return 1;
	}

	return 0;
}

static int parse_strikethrough(const char *buf, size_t len,
                               int *attrs_on, int *attrs_off, int *color)
{
	(void) color;

	strip_quotes(&buf, &len);

	if (!strncasecmp(buf, "true", len)) {
		*attrs_on |= GP_MARKUP_STRIKE;
		return 1;
	}

	if (!strncasecmp(buf, "false", len)) {
		*attrs_off |= GP_MARKUP_STRIKE;
		return 1;
	}

	return 0;
}

static struct html_color_names {
	const char *name;
	int id;
} html_color_names[] = {
	{"text", GP_MARKUP_DEFAULT},
	{"red", GP_MARKUP_RED},
	{"green", GP_MARKUP_GREEN},
	{"yellow", GP_MARKUP_YELLOW},
	{"blue", GP_MARKUP_BLUE},
	{"magenta", GP_MARKUP_MAGENTA},
	{"cyan", GP_MARKUP_CYAN},
	{"gray", GP_MARKUP_GRAY},
	{"bright-red", GP_MARKUP_BR_RED},
	{"bright-green", GP_MARKUP_BR_GREEN},
	{"bright-yellow", GP_MARKUP_BR_YELLOW},
	{"bright-blue", GP_MARKUP_BR_BLUE},
	{"bright-magenta", GP_MARKUP_BR_MAGENTA},
	{"bright-cyan", GP_MARKUP_BR_CYAN},
	{"white", GP_MARKUP_WHITE},
};

static int parse_color(const char *buf, size_t len,
                       int *attrs_on, int *attrs_off, int *color)
{
	(void) attrs_on;
	(void) attrs_off;

	size_t i;

	strip_quotes(&buf, &len);

	for (i = 0; i < GP_ARRAY_SIZE(html_color_names); i++) {
		if (!strncasecmp(buf, html_color_names[i].name, len)) {
			*color = html_color_names[i].id;
			return 1;
		}
	}

	return 0;
}

static struct html_tag_attr {
	const char *name;
	int (*attr_val_parse)(const char *buf, size_t len, int *attrs_on, int *attrs_off, int *color);
} html_tag_attrs[] = {
	{"font_weight", parse_font_weight},
	{"strikethrough", parse_strikethrough},
	{"color", parse_color},
	{"fgcolor", parse_color},
	{"foreground", parse_color},
};

static void parse_attr(struct html_tag_res *tag, int *attrs_on, int *attrs_off, int *color)
{
	unsigned int i;
	int (*attr_val_parse)(const char *buf, size_t len, int *attrs_on, int *attrs_off, int *color) = NULL;
	struct sub_str *attrs = &tag->attrs;
	const char *start;

	html_eat_ws(attrs);

	start = attrs->first;

	while (!is_space(*attrs->first) && *attrs->first != '=' && sub_str_nempty(attrs))
		attrs->first++;

	for (i = 0; i < GP_ARRAY_SIZE(html_tag_attrs); i++) {
		if (!strncasecmp(html_tag_attrs[i].name, start, attrs->first - start)) {
			attr_val_parse = html_tag_attrs[i].attr_val_parse;
		}
	}

	html_eat_ws(attrs);

	if (*attrs->first != '=')
		return;

	attrs->first++;

	html_eat_ws(attrs);

	start = attrs->first;

	while (!is_space(*attrs->first) && sub_str_nempty(attrs))
		attrs->first++;

	if (attr_val_parse)
		attr_val_parse(start, attrs->first - start, attrs_on, attrs_off, color);

	html_eat_ws(attrs);
}

static void parse_attrs(struct html_tag_res *tag, struct attr_stack *attrs)
{
	int attrs_on = 0, attrs_off = 0, color = -1;

	while (sub_str_nempty(&tag->attrs))
		parse_attr(tag, &attrs_on, &attrs_off, &color);

	attr_stack_push(attrs, attrs_on, attrs_off, color);
}

static void eat_until_tag(struct sub_str *markup, const char *tag)
{
	const char *tag_last, *tag_first;

	do {
		while (sub_str_nempty(markup) && *markup->first != '<')
			markup->first++;

		tag_first = ++markup->first;

		while (sub_str_nempty(markup) && *markup->first != '>')
			markup->first++;

		tag_last = markup->first;

		if (sub_str_empty(markup))
			return;
	} while (strncasecmp(tag, tag_first, tag_last - tag_first));

	if (sub_str_nempty(markup))
		markup->first++;
}

static void html_tag(gp_markup_builder *builder, struct sub_str *markup,
                     struct attr_stack *attrs)
{
	int is_closing = 0;
	struct html_tag_res res;

	if (*markup->first == '/') {
		is_closing = 1;
		markup->first++;
	}

	const char *tag_first = markup->first;

	while (sub_str_nempty(markup) && *markup->first != '>')
		markup->first++;

	if (sub_str_empty(markup))
		return;

	const char *tag_last = markup->first;

	markup->first++;

	res = parse_tag(tag_first, tag_last, is_closing);

	switch (res.tag) {
	case HTML_B:
		attr_bit(attrs, is_closing, GP_MARKUP_BOLD);
	break;
	case HTML_BR:
		gp_markup_builder_newline(builder);
	break;
	case HTML_BIG:
		attr_bit(attrs, is_closing, GP_MARKUP_LARGE);
	break;
	case HTML_HR:
		gp_markup_builder_hline(builder);
	break;
	case HTML_P:
		if (is_closing)
			gp_markup_builder_newline(builder);
	break;
	case HTML_S:
		attr_bit(attrs, is_closing, GP_MARKUP_STRIKE);
	break;
	case HTML_SCRIPT:
		eat_until_tag(markup, "/script");
	break;
	case HTML_SPAN:
		if (is_closing)
			attr_stack_pop(attrs);
		else
			parse_attrs(&res, attrs);
	break;
	case HTML_STYLE:
		eat_until_tag(markup, "/style");
	break;
	case HTML_SUB:
		if (!(attr_stack_fmt(attrs) & GP_MARKUP_SUP))
			attr_bit(attrs, is_closing, GP_MARKUP_SUB);
	break;
	case HTML_SUP:
		if (!(attr_stack_fmt(attrs) & GP_MARKUP_SUB))
			attr_bit(attrs, is_closing, GP_MARKUP_SUP);
	break;
	case HTML_U:
		attr_bit(attrs, is_closing, GP_MARKUP_UNDERLINE);
	break;
	case HTML_TITLE:
		if (is_closing)
			gp_markup_builder_newline(builder);
	break;
	case HTML_TT:
		attr_bit(attrs, is_closing, GP_MARKUP_MONO);
	break;
	case HTML_XML:
		eat_until_tag(markup, "/xml");
	break;
	case HTML_UNKNOWN:
	break;
	}
};

static void html_str(gp_markup_builder *builder, struct sub_str *markup, struct attr_stack *attrs)
{
	const char *str_start = markup->first;

	while (sub_str_nempty(markup)) {
		if (is_space(*markup->first) || *markup->first == '<')
			goto ret;
		markup->first++;
	}
ret:
	parse_markup_string(builder, str_start, markup->first, attrs);
}

static void html_parse_ws(struct sub_str *substr, gp_markup_builder *builder,
                          struct attr_stack *attrs, int flags)
{
	int add_space = 0;

	while (sub_str_nempty(substr)) {
		if (!is_space(*substr->first))
			break;

		if ((flags & GP_MARKUP_HTML_KEEP_WS)) {
			switch (*substr->first) {
			case '\n':
				gp_markup_builder_newline(builder);
			break;
			case ' ':
				gp_markup_builder_space(builder, attr_stack_fmt(attrs));
			break;
			case '\t':
				gp_markup_builder_space(builder, attr_stack_fmt(attrs));
				gp_markup_builder_space(builder, attr_stack_fmt(attrs));
				gp_markup_builder_space(builder, attr_stack_fmt(attrs));
				gp_markup_builder_space(builder, attr_stack_fmt(attrs));
			break;
			}
		}

		substr->first++;
		add_space = 1;
	}

	if (!(flags & GP_MARKUP_HTML_KEEP_WS) && add_space)
		gp_markup_builder_space(builder, attr_stack_fmt(attrs));
}

static void parse_markup(gp_markup_builder *builder, struct sub_str *sub_markup, int flags)
{
	struct sub_str markup = *sub_markup;
	struct attr_stack attrs = {};

	while (sub_str_nempty(&markup)) {

		if (is_space(*markup.first))
			html_parse_ws(&markup, builder, &attrs, flags);

		switch (*markup.first) {
		case '<':
			markup.first++;
			html_tag(builder, &markup, &attrs);
		break;
		case 0:
			return;
		default:
			html_str(builder, &markup, &attrs);
		break;
		}
	}
}

static void trim_whitespaces(const char *markup, struct sub_str *substr)
{
	while (is_space(*markup))
		markup++;

	substr->first = markup;

	while (*markup)
		markup++;

	if (markup == substr->first) {
		substr->last = substr->first;
		return;
	}

	do
		markup--;
	while (markup > substr->first && is_space(*markup));

	substr->last = markup;
}

gp_markup *gp_markup_html_parse(const char *markup, enum gp_markup_flags flags)
{
	gp_markup_builder builder = {};
	struct sub_str sub_markup;
	struct gp_markup *ret;

	if (flags & ~(GP_MARKUP_HTML_KEEP_WS)) {
		GP_WARN("Invalid flags");
		return NULL;
	}

	trim_whitespaces(markup, &sub_markup);

	parse_markup(&builder, &sub_markup, flags);

	ret = gp_markup_builder_alloc(&builder);
	if (!ret)
		return NULL;

	parse_markup(&builder, &sub_markup, flags);

	gp_markup_builder_finish(&builder);

	return ret;
}
