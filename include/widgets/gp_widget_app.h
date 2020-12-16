//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_WIDGET_APP_H__
#define GP_WIDGET_APP_H__

#include <widgets/gp_widget_types.h>

/**
 * @brief Loads a widget layout given application name.
 *
 * Looks for the layout in /etc/ and $HOME/.config/
 *
 * @app_name An application name, usually the same as the binary name.
 * @uids An pointer to store the hash table of UIDs to.
 *
 * @return An application widget layout or NULL in a case of failure.
 */
gp_widget *gp_app_layout_load(const char *app_name, void **uids);

#endif /* GP_WIDGET_APP_H__ */
