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

#include <string.h>

#include <GP_Debug.h>

#include "GP_FilterParam.h"

static unsigned int count_channels(GP_FilterParam params[])
{
	unsigned int i = 0;
	
	while (params[i].channel_name[0] != '\0')
		i++;

	return i;
}

GP_FilterParam *GP_FilterParamChannel(GP_FilterParam params[],
                                      const char *channel_name)
{
	unsigned int i;

	for (i = 0; params[i].channel_name[0] != '\0'; i++)
		if (!strcmp(params[i].channel_name, channel_name))
			return &params[i];

	return NULL;
}

uint32_t GP_FilterParamChannels(GP_FilterParam params[])
{
	return count_channels(params);
}

int GP_FilterParamCheckPixelType(GP_FilterParam params[],
                                 GP_PixelType pixel_type)
{
	unsigned int i, num_channels;
	const GP_PixelTypeChannel *channels;
	
	num_channels = GP_PixelTypes[pixel_type].numchannels;
	channels     = GP_PixelTypes[pixel_type].channels;

	i = count_channels(params);

	if (i != num_channels)
		return 1;

	for (i = 0; i < num_channels; i++)
		if (GP_FilterParamChannel(params, channels[i].name) == NULL)
			return 1;
	
	return 0;
}

int GP_FilterParamCheckChannels(GP_FilterParam params[],
                                const char *channel_names[])
{
	unsigned int i;

	for (i = 0; channel_names[i] != NULL; i++)
		if (GP_FilterParamChannel(params, channel_names[i]) == NULL)
			return 1;
	
	if (i != count_channels(params))
		return 1;

	return 0;
}

void GP_FilterParamInitChannels(GP_FilterParam params[],
                                GP_PixelType pixel_type)
{
	unsigned int i, num_channels;
	const GP_PixelTypeChannel *channels;
	
	num_channels = GP_PixelTypes[pixel_type].numchannels;
	channels     = GP_PixelTypes[pixel_type].channels;
	
	for (i = 0; i < num_channels; i++) {
		strcpy(params[i].channel_name, channels[i].name);
		memset(&params[i].val, 0, sizeof(GP_FilterParamVal));
	}
	
	params[i].channel_name[0] = '\0';
}

void GP_FilterParamSetIntAll(GP_FilterParam params[],
                             int32_t val)
{
	unsigned int i;

	for (i = 0; params[i].channel_name[0] != '\0'; i++)
		params[i].val.i = val;
}

void GP_FilterParamSetFloatAll(GP_FilterParam params[],
                               float val)
{
	unsigned int i;

	for (i = 0; params[i].channel_name[0] != '\0'; i++)
		params[i].val.f = val;
}

void GP_FilterParamSetUIntAll(GP_FilterParam params[],
                              uint32_t val)
{
	unsigned int i;

	for (i = 0; params[i].channel_name[0] != '\0'; i++)
		params[i].val.ui = val;
}

void GP_FilterParamSetPtrAll(GP_FilterParam params[],
                             void *ptr)
{
	unsigned int i;

	for (i = 0; params[i].channel_name[0] != '\0'; i++)
		params[i].val.ptr = ptr;
}
