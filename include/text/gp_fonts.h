// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef TEXT_GP_FONTS_H
#define TEXT_GP_FONTS_H

#include <text/gp_font.h>

extern const gp_font_face *const gp_font_gfxprim_mono;
extern const gp_font_face *const gp_font_gfxprim;

/*
 * Monospace 7x6 font
 *
 * Ascend: 5
 * Descent: 2
 * Width: 5 + 1
 */
extern const gp_font_face *gp_font_tiny_mono;

/*
 * Similar as FontTinyMono but proportional
 */
extern const gp_font_face *gp_font_tiny;

/*
 * Monospace 8x8 Commodore 64 like font.
 */
extern const gp_font_face *gp_font_c64;

/*
 * HaxorNarrow family, converted from bdf fonts from:
 *
 * https://github.com/metan-ucw/fonts
 */
extern const gp_font_face *gp_font_haxor_narrow_15;
extern const gp_font_face *gp_font_haxor_narrow_bold_15;
extern const gp_font_face *gp_font_haxor_narrow_16;
extern const gp_font_face *gp_font_haxor_narrow_bold_16;
extern const gp_font_face *gp_font_haxor_narrow_17;
extern const gp_font_face *gp_font_haxor_narrow_bold_17;

#endif /* TEXT_GP_FONTS_H */
