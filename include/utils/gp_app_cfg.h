//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_app_cfg.h
 * @brief Utility functions for application config files.
 */

#ifndef GP_APP_CFG_H
#define GP_APP_CFG_H

#include <stdlib.h>
#include <core/gp_compiler.h>

/**
 * @brief Prepares a path to a config file.
 *
 * The path is constructed as "$HOME/.config/$APP_NAME/$CFG_FILENAME".
 *
 * @param app_name An application name
 * @param cfg_filename A config filename
 * @return A newly allocated buffer with path or NULL in a case of a failure.
 */
char *gp_app_cfg_path(const char *app_name, const char *cfg_filename);

/**
 * @brief Creates a path for a config files, if it does not exists.
 *
 * The path is constructed as "$HOME/.config/$APP_NAME/".
 *
 * Works exactly the same as 'mkdir -p'.
 *
 * @param app_name An application name
 * @return Zero on success, non-zero otherwise and errno is set.
 */
int gp_app_cfg_mkpath(const char *app_name);

/**
 * @brief Does a single scanf() from a config file.
 *
 * Simplified interface for cases when we can read the config with single scanf.
 *
 * @param app_name An application name
 * @param cfg_filename A config filename
 * @param fmt A scanf format string
 * @param ... A scanf parameters
 *
 * @return Returns -1 if the config file does not exists otherwise the return
 *         value from the final scanf().
 */
int gp_app_cfg_scanf(const char *app_name, const char *cfg_filename,
                     const char *fmt, ...) GP_FMT_SCANF(3, 4);

/**
 * @brief Does a simple printf() to a config file.
 *
 * Simplified interface for cases when we can write the config with single
 * printf.
 *
 * @param app_name An application name
 * @param cfg_filename A config filename
 * @param fmt A printf format string
 * @param ... A printf parameters
 *
 * @return Returns -1 on error and errno is set.
 */
int gp_app_cfg_printf(const char *app_name, const char *cfg_filename,
                      const char *fmt, ...) GP_FMT_PRINTF(3, 4);

#endif /* GP_APP_CFG_H */
