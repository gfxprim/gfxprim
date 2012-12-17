/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#ifndef GP_WRITEPIXEL_H
#define GP_WRITEPIXEL_H

#include <stdint.h>
#include <unistd.h>

/*
 * Writes cnt pixels starting at offset off.
 */
void GP_WritePixels_1BPP_LE(uint8_t *start, uint8_t off,
                            size_t cnt, uint8_t val);

/*
 * Writes cnt pixels starting at offset off (offset is in pixel sizes not in
 * bits).
 */
void GP_WritePixels_2BPP_LE(uint8_t *start, uint8_t off,
                            size_t cnt, uint8_t val);

/*
 * Writes cnt pixels starting at offset off (offset is in pixel sizes not in
 * bits i.e. offset could be either 0 or 1).
 */
void GP_WritePixels_4BPP_LE(uint8_t *start, uint8_t off,
                            size_t cnt, uint8_t val);

/*
 * These calls are not byte aligned, thus need start offset.
 */
void GP_WritePixels_1BPP_BE(uint8_t *start, uint8_t off, size_t cnt, uint8_t val);
void GP_WritePixels_2BPP_BE(uint8_t *start, uint8_t off, size_t cnt, uint8_t val);
void GP_WritePixels_4BPP_BE(uint8_t *start, uint8_t off, size_t cnt, uint8_t val);
void GP_WritePixels_18BPP_BE(void *start, uint8_t off, size_t count, uint32_t value);

/* Byte-aligned calls. */
void GP_WritePixels_8BPP(void *start, size_t count, uint8_t value);
void GP_WritePixels_16BPP(void *start, size_t count, uint16_t value);
void GP_WritePixels_24BPP(void *start, size_t count, uint32_t value);
void GP_WritePixels_32BPP(void *start, size_t count, uint32_t value);

/* aliases for older names */
#define GP_WritePixels1bpp GP_WritePixels_1BPP_BE
#define GP_WritePixels2bpp GP_WritePixels_2BPP_BE
#define GP_WritePixels4bpp GP_WritePixels_4BPP_BE
#define GP_WritePixels18bpp GP_WritePixels_18BPP_BE
#define GP_WritePixels8bpp GP_WritePixels_8BPP
#define GP_WritePixels16bpp GP_WritePixels_16BPP
#define GP_WritePixels24bpp GP_WritePixels_24BPP
#define GP_WritePixels32bpp GP_WritePixels_32BPP

#endif /* GP_WRITEPIXEL_H */
