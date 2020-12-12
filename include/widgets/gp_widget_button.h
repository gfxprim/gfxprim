//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_BUTTON_H__
#define GP_WIDGET_BUTTON_H__

#include <widgets/gp_widget_bool.h>

/**
 * @brief Allocates a initialize new button widget.
 *
 * @label A button label.
 * @on_event An widget event handler.
 * @priv User private data.
 *
 * @return A button widget.
 */
gp_widget *gp_widget_button_new(const char *label,
                                int (*on_event)(gp_widget_event *ev),
                                void *priv);

#endif /* GP_WIDGET_BUTTON_H__ */
