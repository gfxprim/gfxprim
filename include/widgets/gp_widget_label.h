//SPDX-License-Identifier: LGPL-2.0-or-later
/*

   Copyright (c) 2014-2024 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_widget_label.h
 * @brief A label widget.
 *
 * A label widget is a single line of a text.
 *
 * @image html label.png
 *
 * A note about the widget size
 * ----------------------------
 *
 * Unless widget 'size' is set the widget grows its size to fit the content
 * without any limitations, which is only useful for cases where the label
 * text is under your control and all possible values are known in advance.
 *
 * If you have a label whose text is outside of your control, e.g. the value is
 * a result of some system call, RPC call, etc, it's important to set the label
 * size so that the label will not grow undefinitelly. The most useful
 * combination is to set both label 'size' along with 'hfill' alignment in
 * which case the label will fill all available horizontal space and the 'size'
 * will function as a minimal widget size. The text inside of the label widget
 * can be aligned with #gp_widget_tattr text attributes.
 *
 * Label JSON attributes
 * ---------------------
 *
 * |     Attribute      |  Type  | Default | Description                                   |
 * |--------------------|--------|---------|-----------------------------------------------|
 * |    **text**        | string |         | Label text                                    |
 * |    **tattr**       |  tattr |    0    | Text attributes #gp_widget_tattr, e.g. bold   |
 * |    **width**       |   int  |    0    | Label text size                               |
 * |    **frame**       |  bool  |  false  | Draw frame around label, implies min padd = 1 |
 * |    **padd**        |   int  |    0    | Padd inside label on left and right           |
 * |    **bg_color**    | string |   "bg"  | Background #gp_widgets_color                  |
 * |   **text_color**   | string | "text"  | Text #gp_widgets_color                        |
 * | **reverse_colors** |  bool  |  false  | Reverse background and text color             |
 */

#ifndef GP_WIDGET_LABEL_H
#define GP_WIDGET_LABEL_H

#include <stdlib.h>
#include <core/gp_compiler.h>

struct gp_widget_label {
	/* Text buffer */
	char *text;
	/* Text format */
	char *text_fmt;
	/* widget size hints */
	const char *set;
	uint8_t width;
	/* attributes */
	uint8_t padd;
	uint16_t frame:1;
	/* colors */
	uint8_t text_color;
	uint8_t bg_color;
	/* text attributes */
	gp_widget_tattr tattr;
};

/**
 * @brief Sets the label text.
 *
 * @param self A label widget.
 * @param text New widget label text.
 */
void gp_widget_label_set(gp_widget *self, const char *text);

/**
 * @brief Sets the label text format.
 *
 * The text format may include a variable that can be then set.
 *
 * @param self A label widget.
 * @param text_fmt A text format.
 */
void gp_widget_labe_fmt_set(gp_widget *self, const char *text_fmt);

/**
 * @brief Sets label accordingly to text format string.
 *
 * @param self A label widget.
 * @param fmt Printf-like format
 * @param ... Printf-like parameters.
 */
void gp_widget_label_fmt_var_set(gp_widget *self, const char *fmt, ...)
                                 GP_FMT_PRINTF(2, 3);

/**
 * @brief Appends text to the label.
 *
 * @param self A label widget.
 * @param text Text to be appended.
 */
void gp_widget_label_append(gp_widget *self, const char *text);

/**
 * @brief Printf-like function to set label text.
 *
 * @param self A label widget.
 * @param fmt Printf formatting string.
 * @param ... Printf parameters.
 * @return Number of characters printed.
 */
int gp_widget_label_printf(gp_widget *self, const char *fmt, ...)
                           __attribute__((format (printf, 2, 3)));

/**
 * @brief VPrintf-like function to set label text.
 *
 * @param self A label widget.
 * @param fmt Printf formatting string.
 * @param ap Printf parameters.
 */
void gp_widget_label_vprintf(gp_widget *self, const char *fmt, va_list ap);

/**
 * @brief Changes text attributes.
 *
 * @param self A label widget.
 * @param tattr New text attributes.
 */
static inline void gp_widget_label_tattr_set(gp_widget *self, gp_widget_tattr tattr)
{
	GP_WIDGET_TYPE_ASSERT(self, GP_WIDGET_LABEL, );

	self->label->tattr = tattr;

	gp_widget_redraw(self);
}

/**
 * @brief Changes widget minimal width.
 *
 * @param self Pointer to a label widget.
 * @param width New label width, the unit for the width is font characters.
 */
static inline void gp_widget_label_width_set(gp_widget *self, unsigned int width)
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
 * @param tattr Text attributes, e.g. bold.
 * @param width Maximal expected text width, if set to non-zero it's used to
 *              callculate the label size.
 * @return Newly allocated label widget.
 */
gp_widget *gp_widget_label_new(const char *text, gp_widget_tattr tattr, unsigned int width);

/**
 * @brief Printf-like function to create a label widget.
 *
 * @param tattr Text attributes, e.g. bold.
 * @param fmt Printf formatting string.
 * @param ... Printf parameters.
 * @return Newly allocated label widget.
 */
gp_widget *gp_widget_label_printf_new(gp_widget_tattr tattr, const char *fmt, ...)
                                      __attribute__((format (printf, 2, 3)));

#endif /* GP_WIDGET_LABEL_H */
