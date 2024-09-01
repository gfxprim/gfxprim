//SPDX-License-Identifier: LGPL-2.0-or-later
/*

  Copyright (C) 2022 Cyril Hrubis <metan@ucw.cz>

*/

/**
 * @file gp_vec_str.h
 * @brief A dynamic C strings.
 */

#ifndef GP_VEC_STR_H
#define GP_VEC_STR_H

#include <string.h>
#include <core/gp_compiler.h>
#include <utils/gp_utf.h>
#include <utils/gp_vec.h>

/**
 * @brief Allocated new string vector.
 *
 * @return Returns newly allocated empty string vector or a NULL in a case of a
 *         failure.
 */
static inline char *gp_vec_str_new(void)
{
	return gp_vec_new(1, 1);
}

/**
 * @brief Returns string vector len.
 *
 * @param self A string vector.
 *
 * @return Returns string vector length.
 */
static inline size_t gp_vec_strlen(const char *self)
{
	return gp_vec_len(self) - 1;
}

/**
 * @brief Returns string vector size.
 *
 * @param self A string vector.
 *
 * @return Returns string vector size i.e. length + 1.
 */
static inline size_t gp_vec_strsize(const char *self)
{
	return gp_vec_len(self);
}

/**
 * @brief Copies a string into newly allocated string vector.
 *
 * @param src A strint to duplicate.
 *
 * @return A string vector or a NULL in a case of allocation failure.
 */
static inline char *gp_vec_strdup(const char *src)
{
	char *ret = gp_vec_new(strlen(src)+1, 1);

	if (!ret)
		return NULL;

	strcpy(ret, src);

	return ret;
}

/**
 * @brief Inserts a string into a string vector at a given offset.
 *
 * @param self A string vector.
 * @param off  An offset in the string vector.
 * @param src  A string to copy.
 *
 * @return A string vector or a NULL in a case of allocation failure.
 */
GP_WUR static inline char *gp_vec_strins(char *self, size_t off, const char *src)
{
	char *ret = gp_vec_ins(self, off, strlen(src));

	if (!ret)
		return NULL;

	memcpy(ret+off, src, strlen(src));

	return ret;
}

/**
 * @brief Appends a string to a string vector.
 *
 * @param self A string vector.
 * @param str A string to append.
 *
 * @return A string vector or a NULL in case of an allocation failure.
 */
GP_WUR static inline char *gp_vec_str_append(char *self, const char *str)
{
	return gp_vec_strins(self, gp_vec_len(self)-1, str);
}

#define GP_VEC_STR_APPEND(self, str) ({\
		char *gp_ret__ = gp_vec_str_append(self, str); \
		if (gp_ret__) \
			self = gp_ret__; \
		gp_ret__; \
	})

/**
 * @brief Inserts a string into a string vector at a given offset.
 *
 * @param self A string vector.
 * @param off  An offset in the string vector.
 * @param src  A string to copy.
 *
 * @return A string vector or a NULL in a case of allocation failure.
 */
GP_WUR static inline char *gp_vec_chins(char *self, size_t off, char ch)
{
	char *ret = gp_vec_ins(self, off, 1);

	if (!ret)
		return NULL;

	ret[off] = ch;

	return ret;
}

/**
 * @brief Inserts an unicode character into an UTF-8 string.
 *
 * @param self A string vector.
 * @param off An offset in the string.
 * @unicode An unicode character.
 *
 * @return A string vector or a NULL in a case of allocation failure.
 */
GP_WUR static inline char *gp_vec_ins_utf8(char *self, size_t off, uint32_t unicode)
{
	char *ret = gp_vec_ins(self, off, gp_utf8_bytes(unicode));

	if (!ret)
		return NULL;

	gp_to_utf8(unicode, ret+off);

	return ret;
}

/**
 * @brief Deletes len characters from a string vector at a given offset.
 *
 * @param self A string vector.
 * @param off An offset in the string vector.
 * @param len Number of characters to delete.
 *
 * @return Returns NULL if off + len is outside of the vector and the input
 *         vector is untouched. Othervise modified string vector is returned.
 */
GP_WUR
static inline char *gp_vec_strdel(char *self, size_t off, size_t len)
{
	return gp_vec_del(self, off, len);
}

/**
 * @brief Clears the string.
 *
 * @param self A string vector.
 *
 * @return An empty string vector.
 */
GP_WUR static inline char *gp_vec_strclr(char *self)
{
	char *ret = gp_vec_resize(self, 1);

	ret[0] = 0;

	return ret;
}

/**
 * @brief Printf into a string vector.
 *
 * @param self A string vector.
 * @param fmt Printf format string.
 * @param ... Printf parameters.
 *
 * @return A new vector size or -1 on allocation failure.
 */
GP_WUR char *gp_vec_printf(char *self, const char *fmt, ...) GP_FMT_PRINTF(2, 3);

/**
 * @brief Printf va_list into a string vector.
 *
 * @param self A string vector.
 * @param fmt Printf format string.
 * @param va A va list.
 *
 * @return A new vector size or -1 on allocation failure.
 */
GP_WUR char *gp_vec_vprintf(char *self, const char *fmt, va_list va);

#endif	/* GP_VEC_STR_H */
