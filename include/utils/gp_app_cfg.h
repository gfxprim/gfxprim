//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * Utility functions for application config files.
 */

#ifndef GP_APP_CFG_H
#define GP_APP_CFG_H

/**
 * Prepares a path to a config file as "$HOME/.config/$APP_NAME/$CFG_FILENAME".
 *
 * @app_name An application name
 * @cfg_filename A config filename
 * @return A newly allocated buffer with path or NULL in a case of a failure.
 */
char *gp_app_cfg_path(const char *app_name, const char *cfg_filename);

/**
 * Creates a path, if it does not exists, for config files in
 * "$HOME/.config/$APP_NAME/".
 *
 * Works exactly the same as 'mkdir -p'.
 *
 * @app_name An application name
 * @return Zero on success, non-zero otherwise and errno is set.
 */
int gp_app_cfg_mkpath(const char *app_name);

/**
 * Simplified interface for cases when we can read the config with single scanf.
 *
 * @app_name An application name
 * @cfg_filename A config filename
 * @fmt A scanf format string
 * @... A scanf parameters
 *
 * @return Returns -1 if the config file does not exists otherwise the return
 *         value from the final scanf().
 */
int gp_app_cfg_scanf(const char *app_name, const char *cfg_filename,
                     const char *fmt, ...) __attribute__ ((format (scanf, 3, 4)));

/**
 * Simplified interface for cases when we can write the config with single printf.
 *
 * @app_name An application name
 * @cfg_filename A config filename
 * @fmt A printf format string
 * @... A printf parameters
 *
 * @return Returns -1 on error and errno is set.
 */
int gp_app_cfg_printf(const char *app_name, const char *cfg_filename,
                      const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

#endif /* GP_APP_CFG_H */
