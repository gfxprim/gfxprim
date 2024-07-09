//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2022-2023 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_path.h
 * @brief Utility functions for building paths.
 */

#ifndef GP_PATH_H
#define GP_PATH_H

/**
 * @brief Returns a user home directory.
 *
 * The home directory is validated to be an absolute path.
 *
 * @return An absolute path to the user home directory or NULL on failure.
 */
char *gp_user_home(void);

char *gp_compose_path_(const char *dir, ...);

/**
 * @brief Builds a path from directories passsed in arguments.
 *
 * The returned path is normalized, i.e. there are no two path delimiters '/'
 * next to each other.
 *
 * @param dir A first directory in path.
 * @param ... A NULL terminated list of path components.
 * @return A newly allocated buffer with path or NULL in a case of a failure.
 */
#define gp_compose_path(dir, ...) gp_compose_path_(dir, ##__VA_ARGS__, NULL);

enum gp_mkpath_flags {
	/** Last component in path if it does not end with slash is ignored */
	GP_MKPATH_HAS_FILE = 0x01,
};

/**
 * @brief Creates a path, if it does not exists.
 *
 * Works similar to 'mkdir -p'. Hoever if last compoment of the path is a file,
 * i.e. last path component does not end with '/' it's ignored.
 *
 * @param base_path A path to create the directories under, can be NULL.
 * @param path A path, one or more directories.
 * @param flags An enum gp_mkpath_flags.
 * @param mode A mode for the directories created in the path.
 * @return Zero on success, non-zero otherwise and errno is set.
 */
int gp_mkpath(const char *base_path, const char *path,
              enum gp_mkpath_flags flags, int mode);

/**
 * @brief Returns directory component of a path in a newly allocated buffer.
 *
 * @param path A path.
 * @return A newly allocated buffer with a directory component of the path.
 *         Returns NULL if path haven't included a directory component or
 *         if an allocation has failed.
 */
char *gp_dirname(const char *path);

#endif /* GP_PATH_H */
