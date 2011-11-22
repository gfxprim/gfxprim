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
 * Copyright (C) 2009-2011 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Filter per channel parameter passing code.

 */

#ifndef FILTERS_GP_FILTER_PARAM_H
#define FILTERS_GP_FILTER_PARAM_H

#include "core/GP_Pixel.h"

#include <stdint.h>

typedef union GP_FilterParamVal {
	float f;
	uint32_t ui;
	int32_t i;
	void *ptr;
} GP_FilterParamVal;

/*
 * Filter parameter structure for one channel.
 *
 * Filter takes, empty channel name terminated, arrray of these as parameter.
 */
typedef struct GP_FilterParam {
	//TODO: this must be >= for maximal channel name (now it's 2)
	char channel_name[2];
	union GP_FilterParamVal val;
} GP_FilterParam;

/*
 * Takes array of filter parameters and returns channel.
 *
 * Returns NULL if channel wasn't found.
 */
GP_FilterParam *GP_FilterParamChannel(GP_FilterParam params[],
                                      const char *channel_name);

/*
 * Returns number of filter channels.
 */
uint32_t GP_FilterParamChannels(GP_FilterParam params[]);

/*
 * Compares param channels and pixel type channels. Returns zero if channels
 * match.
 */
int GP_FilterParamCheckPixelType(GP_FilterParam params[],
                                 GP_PixelType type);

/*
 * Returns zero only if params have exactly same channels as array of
 * channel_names.
 */
int GP_FilterParamCheckChannels(GP_FilterParam params[],
                                const char *channel_names[]);

/*
 * Create and initalize the structure on the stack
 */
#define GP_FILTER_PARAMS(pixel_type, name) \
	GP_FilterParam name[GP_PixelTypes[pixel_type].numchannels + 1]; \
	GP_FilterParamInitChannels(name, pixel_type);
/*
 * Initalize param names and terminator.
 *
 * Sets all values to 0.
 */
void GP_FilterParamInitChannels(GP_FilterParam params[],
                                GP_PixelType type);

/*
 * Sets all values to integer value.
 */
void GP_FilterParamSetIntAll(GP_FilterParam params[],
                             int32_t val);

/*
 * Sets all values to float value.
 */
void GP_FilterParamSetFloatAll(GP_FilterParam params[],
                               float val);

#endif /* FILTERS_GP_FILTER_PARAM_H */
