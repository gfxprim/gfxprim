//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * Utility function to open config file.
 */

#ifndef GP_WIDGET_CFG_H
#define GP_WIDGET_CFG_H

/**
 * Prepares a path to a config file as "$HOME/.config/$APP_NAME/$CFG_FILENAME".
 *
 * @app_name An application name
 * @cfg_filename A config filename
 * @return A newly allocated buffer with path or NULL in a case of a failure.
 */
char *gp_widget_cfg_path(const char *app_name, const char *cfg_filename);

/**
 * Creates a path, if it does not exists, for config files in
 * "$HOME/.config/$APP_NAME/".
 *
 * Works exactly the same as 'mkdir -p'.
 *
 * @app_name An application name
 * @return Zero on success, non-zero otherwise and errno is set.
 */
int gp_widget_cfg_mkpath(const char *app_name);

#endif /* GP_WIDGET_CFG_H */
