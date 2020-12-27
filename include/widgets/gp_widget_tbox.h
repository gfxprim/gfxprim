//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TBOX_H__
#define GP_WIDGET_TBOX_H__

struct gp_widget_tbox {
	/*
	 * If not NULL the tbox can contain only characters from this
	 * string, this is used as a hint when minimal tbox size is
	 * accounted for.
	 */
	const char *filter;

	int hidden:1; /* password prompt */
	int alert:1;

	size_t max_size;
	size_t cur_pos;
	size_t size;
	char *buf;

	size_t off_left;

	char payload[];
};

enum gp_widget_tbox_flags {
	GP_WIDGET_TBOX_HIDDEN = 0x01,
};

#define GP_TBOX_FILTER_INT "0123456789"
#define GP_TBOX_FILTER_HEX "0123456789abcdefABCDEF"

/**
 * @brief Allocate and initialize a tbox widget.
 *
 * @text     Initial tbox text.
 * @length   Expected text length, used for widget computation.
 * @filter   If set only characters from the string can be typed into the tbox.
 * @on_event Callback called on tbox event, e.g. key press.
 * @priv     User private pointer.
 * @flags    Mask of enum gp_widget_tbox_flags.
 *
 * @returns A tbox widget.
 */
gp_widget *gp_widget_tbox_new(const char *text, unsigned int length,
                                 const char *filter,
                                 int (*on_event)(gp_widget_event *),
                                 void *priv, int flags);

/**
 * @brief A printf-like function to set a tbox value.
 *
 * @self A tbox widget.
 * @fmt A printf-like format string.
 * @... A printf-like parameters.
 *
 * @return A tbox string lenght or -1 on a failure.
 */
int gp_widget_tbox_printf(gp_widget *self, const char *fmt, ...)
                          __attribute__((format (printf, 2, 3)));

/**
 * @brief Clears the tbox text.
 *
 * @self A tbox widget.
 */
void gp_widget_tbox_clear(gp_widget *self);


/**
 * @brief Returns a tbox string.
 *
 * @self A tbox widget.
 * @return Texbox widget string.
 */
const char *gp_widget_tbox_str(gp_widget *self);

#endif /* GP_WIDGET_TBOX_H__ */
