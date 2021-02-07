//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2014-2020 Cyril Hrubis <metan@ucw.cz>

 */

#ifndef GP_STRING_H
#define GP_STRING_H

/**
 * @brief Computes sum of sizes needed for a deep copy of an array of strings.
 *
 * @param strings An array of strings.
 * @param len Length of the string array.
 * @return Size needed for a deep copy of the string array.
 */
size_t gp_string_arr_size(const char *strings[], unsigned int len);

/**
 * @brief Copies an array of strings into a buffer.
 *
 * @param strings An array of strings.
 * @param len Length of the string array.
 * @buf Buffer large enough to fit the copy.
 * @return A deep copy of the string array.
 */
char **gp_string_arr_copy(const char *strings[], unsigned int len, void *buf);


/**
 * @brief Printf that allocates suitable buffer.
 *
 * @param fmt printf format string
 * @param ... parameters
 * @return Newly allocated string that should be freed later on.
 */
char *gp_aprintf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif /* GP_STRING_H */
