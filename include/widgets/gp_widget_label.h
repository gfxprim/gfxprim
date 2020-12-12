//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_LABEL_H__
#define GP_WIDGET_LABEL_H__

#include <stdlib.h>

struct gp_widget_label {
	/* Text buffer */
	char *text;
	/* widget size hints */
	const char *set;
	unsigned int width:8;
	/* attributes */
	unsigned int bold:1;
	unsigned int ralign:1;
	unsigned int frame:1;
};

/**
 * @brief Sets the label text.
 *
 * @param self Pointer to a label widget.
 * @param text New widget label text.
 */
void gp_widget_label_set(gp_widget *self, const char *text);

/**
 * @brief Printf-like function to set label text.
 * @param self Pointer to a label widget.
 * @param fmt  Printf formatting string.
 * @param ...  Printf parameters.
 * @return Number of characters printed.
 */
int gp_widget_label_printf(gp_widget *self, const char *fmt, ...)
                           __attribute__((format (printf, 2, 3)));

/**
 * @brief Turns on-off bold text attribute.
 */
static inline void gp_widget_label_bold(gp_widget *self, int bold)
{
	GP_WIDGET_ASSERT(self, GP_WIDGET_LABEL, );

	self->label->bold = bold;

	gp_widget_redraw(self);
}

/**
 * @brief Changes widget minimal width.
 *
 * @self Pointer to a label widget.
 * @width New label width, the unit for the width is font characters.
 */
static inline void gp_widget_label_set_width(gp_widget *self, unsigned int width)
{
	self->label->width = width;
	gp_widget_resize(self);
}

/**
 * @brief Allocates a label widget.
 *
 * Label minimal width is a bit more complicated topic since labels can change
 * content and hence size dynamically. There are two different modes of
 * operation.
 *
 * - If width is set, the size is set to stone so that the label can hold width
 *   characters. The main problem is that for proprotional fonts the allocated
 *   space is huge because we account for maximal character width. To
 *   accomodate for that you can also pass a string with subset of characters
 *   that are expected to appear in the label to make that computation more
 *   precise.
 *
 * - If width is not set, i.e. set to zero, the size is computed accordintly to
 *   the current label text, however by default the widget does not shrink,
 *   e.g. if the new string width is shorter than the last one we do not
 *   trigger resize, which avoids for the layout to size to jump on label text
 *   changes. You can manually trigger shrinking by calling gp_widget_resize().
 *
 * @param text A label text.
 * @param width Maximal expected text width, if set to non-zero it's used to
 *              callculate the label size.
 * @param bold Sets the bold text attribute.
 * @return Newly allocated label widget.
 */
gp_widget *gp_widget_label_new(const char *text, unsigned int width, int bold);

/**
 * @brief Printf-like function to create a label widget.
 *
 * @param bold Sets the bold text attribute.
 * @param fmt Printf formatting string.
 * @param ... Printf parameters.
 * @return Newly allocated label widget.
 */
gp_widget *gp_widget_label_printf_new(int bold, const char *fmt, ...)
                                      __attribute__((format (printf, 2, 3)));

#endif /* GP_WIDGET_LABEL_H__ */
