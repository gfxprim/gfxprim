//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

/*
 * Utility functions for building paths.
 */

#ifndef GP_USER_PATH_H
#define GP_USER_PATH_H

#include <utils/gp_path.h>

/**
 * Prepares a buffer with "$HOME/$PATH/$FILE_NAME" path.
 *
 * @path A path, one or more directories.
 * @file_name A file name.
 * @return A newly allocated buffer with path or NULL in a case of a failure.
 */
char *gp_user_path(const char *path, const char *file_name);

/**
 * Creates a path, if it does not exists, for config files in "$HOME/$PATH/".
 *
 * Works exactly the same as 'mkdir -p'.
 *
 * @path A path, one or more directories.
 * @return Zero on success, non-zero otherwise and errno is set.
 */
int gp_user_mkpath(const char *path, enum gp_mkpath_flags flags);

#endif /* GP_USER_PATH_H */
