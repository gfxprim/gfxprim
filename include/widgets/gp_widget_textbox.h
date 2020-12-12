//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_TEXT_BOX_H__
#define GP_WIDGET_TEXT_BOX_H__

struct gp_widget_textbox {
	/*
	 * If not NULL the textbox can contain only characters from this
	 * string, this is used as a hint when minimal textbox size is
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

enum gp_widget_text_box_flags {
	GP_WIDGET_TEXT_BOX_HIDDEN = 0x01,
};

#define GP_TEXT_BOX_FILTER_INT "0123456789"
#define GP_TEXT_BOX_FILTER_HEX "0123456789abcdefABCDEF"

/**
 * @brief Allocate and initialize a textbox widget.
 *
 * @text     Initial textbox text.
 * @length   Expected text length, used for widget computation.
 * @filter   If set only characters from the string can be typed into the textbox.
 * @on_event Callback called on textbox event, e.g. key press.
 * @priv     User private pointer.
 * @flags    Mask of enum gp_widget_text_box_flags.
 *
 * @returns A textbox widget.
 */
gp_widget *gp_widget_textbox_new(const char *text, unsigned int length,
                                 const char *filter,
                                 int (*on_event)(gp_widget_event *),
                                 void *priv, int flags);

/**
 * @brief A printf-like function to set a textbox value.
 *
 * @self A textbox widget.
 * @fmt A printf-like format string.
 * @... A printf-like parameters.
 *
 * @return A textbox string lenght or -1 on a failure.
 */
int gp_widget_textbox_printf(gp_widget *self, const char *fmt, ...)
                             __attribute__((format (printf, 2, 3)));

/**
 * @brief Clears the textbox text.
 *
 * @self A textbox widget.
 */
void gp_widget_textbox_clear(gp_widget *self);


/**
 * @brief Returns a textbox string.
 *
 * @self A textbox widget.
 * @return Texbox widget string.
 */
const char *gp_widget_textbox_str(gp_widget *self);

#endif /* GP_WIDGET_TEXT_BOX_H__ */
