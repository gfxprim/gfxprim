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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <math.h>

#include "GP_Pixel.h"
#include "GP_Debug.h"

#include "GP_Gamma.h"

static GP_GammaTable *tables = NULL;

static void fill_table8(GP_GammaTable *table, float gamma,
                        uint8_t in_bits, uint8_t out_bits)
{
	unsigned int i;
	unsigned int in_max = (1<<in_bits) - 1;
	unsigned int out_max = (1<<out_bits) - 1;

	GP_DEBUG(3, "Initalizing gamma table %f", gamma);

	for (i = 0; i < (1U<<in_bits); i++)
		table->u8[i] = pow((float)i / in_max, gamma) * out_max + 0.5;
}

static void fill_table16(GP_GammaTable *table, float gamma,
                         uint8_t in_bits, uint8_t out_bits)
{
	unsigned int i;
	unsigned int in_max = (1<<in_bits) - 1;
	unsigned int out_max = (1<<out_bits) - 1;

	GP_DEBUG(3, "Initalizing gamma table %f", gamma);

	for (i = 0; i < (1U<<in_bits); i++)
		table->u16[i] = pow((float)i / in_max, gamma) * out_max + 0.5;
}

static GP_GammaTable *get_table(float gamma, uint8_t in_bits, uint8_t out_bits)
{
	GP_GammaTable *i;

	for (i = tables; i != NULL; i = i->next)
		if (gamma == i->gamma && in_bits == i->in_bits &&
		    out_bits == i->out_bits)
			break;

	if (i != NULL) {
		GP_DEBUG(2, "Found Gamma table Gamma %f, in_bits %u, "
		         "out_bits %u, ref_count %u", i->gamma, i->in_bits,
			 i->out_bits, i->ref_count);
		i->ref_count++;
		return i;
	}

	GP_DEBUG(2, "Creating Gamma table Gamma %f, in_bits %u, out_bits %u",
	         gamma, in_bits, out_bits);

	i = malloc(sizeof(GP_GammaTable) + (1U<<in_bits) * (out_bits > 8 ? 2 : 1));

	if (i == NULL) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	i->gamma = gamma;
	i->in_bits = in_bits;
	i->out_bits = out_bits;
	i->ref_count = 1;
	i->type = GP_CORRECTION_GAMMA;

	if (out_bits > 8)
		fill_table16(i, gamma, in_bits, out_bits);
	else
		fill_table8(i, gamma, in_bits, out_bits);

	/* Insert it into link list */
	i->next = tables;
	tables = i;

	return i;
}

static void put_table(GP_GammaTable *table)
{
	if (table == NULL)
		return;

	table->ref_count--;

	GP_DEBUG(2, "Putting gamma table Gamma %f, in_bits %u, out_bits %u, "
	         "ref_count %u", table->gamma, table->in_bits, table->out_bits,
		 table->ref_count);

	if (table->ref_count == 0) {
		GP_DEBUG(2, "Gamma table ref_count == 0, removing...");

		GP_GammaTable *i, *prev = NULL;

		/* Remove from link list and free */
		for (i = tables; i != NULL; i = i->next) {
			if (table == i)
				break;
			prev = i;
		}

		if (prev == NULL)
			tables = table->next;
		else
			prev->next = table->next;

		free(table);
	}
}

GP_Gamma *GP_GammaAcquire(GP_PixelType pixel_type, float gamma)
{
	GP_CHECK_VALID_PIXELTYPE(pixel_type);
	int channels = GP_PixelTypes[pixel_type].numchannels, i;

	GP_DEBUG(1, "Acquiring Gamma table %s gamma %f", GP_PixelTypeName(pixel_type), gamma);

	GP_Gamma *res = malloc(sizeof(struct GP_Gamma) + 2 * channels * sizeof(void*));

	if (res == NULL) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	/* NULL the pointers */
	for (i = 0; i < 2 * channels; i++)
		res->tables[i] = NULL;

	res->pixel_type = pixel_type;
	res->ref_count = 1;

	/* Gamma to linear tables n bits -> n + 2 bits */
	for (i = 0; i < channels; i++) {
		unsigned int chan_size = GP_PixelTypes[pixel_type].channels[i].size;
		res->tables[i] = get_table(gamma, chan_size, chan_size + 2);

		if (res->tables[i] == NULL) {
			GP_GammaRelease(res);
			return NULL;
		}
	}

	/* And reverse tables, n + 2 bits -> n bits */
	for (i = 0; i < channels; i++) {
		unsigned int chan_size = GP_PixelTypes[pixel_type].channels[i].size;
		res->tables[i + channels] = get_table(1/gamma, chan_size + 2, chan_size);

		if (res->tables[i] == NULL) {
			GP_GammaRelease(res);
			return NULL;
		}
	}

	return res;
}

GP_Gamma *GP_GammaCopy(GP_Gamma *self)
{
	self->ref_count++;
	return self;
}

void GP_GammaRelease(GP_Gamma *self)
{
	int channels, i;

	if (!self)
		return;

	channels = GP_PixelTypes[self->pixel_type].numchannels;

	GP_DEBUG(1, "Releasing Gamma table %s gamma %f", GP_PixelTypeName(self->pixel_type), self->tables[0]->gamma);

	for (i = 0; i < channels; i++)
		put_table(self->tables[i]);

	if (--self->ref_count == 0) {
		GP_DEBUG(2, "Gamma ref_count == 0, releasing...");
		free(self);
	}
}

static const char *correction_type_names[] = {
	"Gamma",
	"sRGB",
};

static const char *correction_type_name(enum GP_CorrectionType type)
{
	if (type > GP_CORRECTION_sRGB)
		return "Invalid";

	return correction_type_names[type];
}

void GP_GammaPrint(const GP_Gamma *self)
{
	printf("Correction tables:\n");

	const GP_PixelTypeDescription *desc = GP_PixelTypeDesc(self->pixel_type);

	unsigned int i;

	for (i = 0; i < desc->numchannels; i++) {
		enum GP_CorrectionType type = self->tables[i]->type;

		printf(" %s: %s", desc->channels[i].name,
		       correction_type_name(type));

		if (type == GP_CORRECTION_GAMMA)
			printf(" gamma = %.2f", self->tables[i]->gamma);

		printf("\n");
	}
}
