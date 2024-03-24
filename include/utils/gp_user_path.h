//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_user_path.h
 * @brief Utility functions for building paths.
 */

#ifndef GP_USER_PATH_H
#define GP_USER_PATH_H

#include <utils/gp_path.h>

/**
 * @brief Prepares a buffer with a path.
 *
 * The path is constructed as "$HOME/$PATH/$FILE_NAME".
 *
 * @param path A path, one or more directories.
 * @param file_name A file name.
 * @return A newly allocated buffer with path or NULL in a case of a failure.
 */
char *gp_user_path(const char *path, const char *file_name);

/**
 * @brief Creates a path, if it does not exists.
 *
 * The path is constructed in "$HOME/$PATH/".
 *
 * Works exactly the same as 'mkdir -p'.
 *
 * @param path A path, one or more directories.
 * @return Zero on success, non-zero otherwise and errno is set.
 */
int gp_user_mkpath(const char *path, enum gp_mkpath_flags flags);

#endif /* GP_USER_PATH_H */
