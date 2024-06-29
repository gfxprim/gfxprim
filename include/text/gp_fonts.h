// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_fonts.h
 * @brief A font lookup functions.
 *
 * Functions to lookup compiled-in fonts.
 */
#ifndef TEXT_GP_FONTS_H
#define TEXT_GP_FONTS_H

#include <text/gp_font.h>

extern const gp_font_family *gp_font_family_default;

/**
 * @brief Font family lookup.
 *
 * Looks up a font family by a family name in the compiled in fonts.
 *
 * @param family_name A family name, such as 'gfxprim', 'tiny', etc.
 *
 * @return A pointer to a font family or NULL if not found.
 */
const gp_font_family *gp_font_family_lookup(const char *family_name);

/**
 * @brief Looks up a font face in a family given a style flags.
 *
 * If family pointer is a null default 'gfxprim' family is used instead.
 *
 * @param family A pointer to a font family. If NULL is passed default is used.
 * @param style A bitmask of enum #gp_font_style. If GP_FONT_FALLBACK is passed
 *        the function never returns NULL.
 *
 * @return A pointer to a font or NULL if not found.
 */
const gp_font_face *gp_font_family_face_lookup(const gp_font_family *family, uint8_t style);

/**
 * @brief Looks up a font face in a family given a family name and style flags.
 *
 * @param family_name A family name such as 'gfxprim', 'tiny', etc.
 * @param style A bitmask of enum gp_font_style. If GP_FONT_FALLBACK is passed
 *              the function never returns NULL.
 *
 * @return A pointer to a font family or NULL if not found.
 */
const gp_font_face *gp_font_face_lookup(const char *family_name, uint8_t style);

/**
 * @brief Font family iterator.
 */
typedef struct gp_fonts_iter {
	int16_t family_idx;
	int16_t font_idx;
} gp_fonts_iter;

/**
 * @brief An iterator direction.
 */
enum gp_fonts_iter_dir {
	/** @brief No action. */
	GP_FONTS_ITER_NOP = 0,
	/** @brief Move to previous. */
	GP_FONTS_ITER_PREV = -1,
	/** @brief Move to next. */
	GP_FONTS_ITER_NEXT = 1,
	/** @brief Move to first. */
	GP_FONTS_ITER_FIRST = -2,
	/** @brief Move to last. */
	GP_FONTS_ITER_LAST = 2,
};

/**
 * @brief Iterates over all compiled in fonts.
 *
 * @param iter An iterator.
 * @param wrap If non zero makes the iteration circular,
 *             i.e. NULL is not returned at the end.
 * @param dir Direction to move to.
 *
 * @return A font face.
 */
const gp_font_face *gp_fonts_iter_font(gp_fonts_iter *iter, int wrap,
                                       enum gp_fonts_iter_dir dir);

/**
 * @brief Iterates over all compiled in families.
 *
 * @param iter An iterator.
 * @param wrap If non zero makes the iteration circular,
 *             i.e. NULL is not returned at the end.
 * @param dir Direction to move to.
 *
 * @return Current font family.
 */
const gp_font_family *gp_fonts_iter_family(gp_fonts_iter *iter, int wrap,
                                           enum gp_fonts_iter_dir dir);

/**
 * @brief A loop over all compiled in fonts.
 */
#define GP_FONT_FAMILY_FOREACH(iter, family) \
	for (family = gp_fonts_iter_family(iter, 0, GP_FONTS_ITER_FIRST); \
	     family; \
	     family = gp_fonts_iter_family(iter, 0, GP_FONTS_ITER_NEXT))

#endif /* TEXT_GP_FONTS_H */
