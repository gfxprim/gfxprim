//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

/*

   Simple markup.

   - variables are enclosed in {0.000}
     the string between {} is used for
     initial size computation

   - newlines are encoded with \n

   - bold is enclosed with asterisks: *bold*

   - escape character is \ as in \* or \{

 */

#ifndef GP_MARKUP_PARSER_H__
#define GP_MARKUP_PARSER_H__

enum gp_markup_elem_type {
	GP_MARKUP_END,
	GP_MARKUP_STR,
	GP_MARKUP_VAR,
	GP_MARKUP_NEWLINE,
};

enum gp_markup_elem_attr {
	GP_MARKUP_BOLD = 0x01,
	GP_MARKUP_BIG = 0x02,
	GP_MARKUP_INVERSE = 0x04,
	GP_MARKUP_SUBSCRIPT = 0x08,
	GP_MARKUP_SUPERSCRIPT = 0x10,
};

typedef struct gp_markup_elem {
	int type:4;
	int attrs:8;

	const char *str;

	char *var;
} gp_markup_elem;

typedef struct gp_markup {
	char *markup;
	struct gp_markup_elem elems[];
} gp_markup;

static inline gp_markup_elem *gp_markup_first(gp_markup *self)
{
	return self->elems;
}

static inline gp_markup_elem *gp_markup_next(gp_markup_elem *elem)
{
	if ((++elem)->type == GP_MARKUP_END)
		return NULL;

	return elem;
}

static inline const char *gp_markup_elem_str(const gp_markup_elem *elem)
{
	switch (elem->type) {
	case GP_MARKUP_STR:
		return elem->str;
	break;
	case GP_MARKUP_VAR:
		if (elem->var)
			return elem->var;
		return elem->str;
	break;
	default:
		return NULL;
	}
}

gp_markup *gp_markup_parse(const char *markup);

static inline gp_markup_elem *gp_markup_next_line(gp_markup_elem *elem)
{
	for (;;) {
		if (elem->type == GP_MARKUP_NEWLINE)
			return elem+1;

		if (elem->type == GP_MARKUP_END)
			return NULL;

		elem++;
	}
}

void gp_markup_free(gp_markup *self);

#endif /* GP_MARKUP_PARSER_H__ */
