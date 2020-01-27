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

#include <core/gp_pixel.h>
#include <core/gp_debug.h>

#include <core/gp_gamma.h>

static gp_gamma_table *tables = NULL;

static void fill_table8(gp_gamma_table *table, float gamma,
                        uint8_t in_bits, uint8_t out_bits)
{
	unsigned int i;
	unsigned int in_max = (1<<in_bits) - 1;
	unsigned int out_max = (1<<out_bits) - 1;

	GP_DEBUG(3, "Initalizing gamma table %f", gamma);

	for (i = 0; i < (1U<<in_bits); i++)
		table->u8[i] = pow((float)i / in_max, gamma) * out_max + 0.5;
}

static void fill_table16(gp_gamma_table *table, float gamma,
                         uint8_t in_bits, uint8_t out_bits)
{
	unsigned int i;
	unsigned int in_max = (1<<in_bits) - 1;
	unsigned int out_max = (1<<out_bits) - 1;

	GP_DEBUG(3, "Initalizing gamma table %f", gamma);

	for (i = 0; i < (1U<<in_bits); i++)
		table->u16[i] = pow((float)i / in_max, gamma) * out_max + 0.5;
}

static gp_gamma_table *get_table(float gamma, uint8_t in_bits, uint8_t out_bits)
{
	gp_gamma_table *i;

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

	i = malloc(sizeof(gp_gamma_table) + (1U<<in_bits) * (out_bits > 8 ? 2 : 1));

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

static void put_table(gp_gamma_table *table)
{
	if (table == NULL)
		return;

	table->ref_count--;

	GP_DEBUG(2, "Putting gamma table Gamma %f, in_bits %u, out_bits %u, "
	         "ref_count %u", table->gamma, table->in_bits, table->out_bits,
		 table->ref_count);

	if (table->ref_count == 0) {
		GP_DEBUG(2, "Gamma table ref_count == 0, removing...");

		gp_gamma_table *i, *prev = NULL;

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

gp_gamma *gp_gamma_acquire(gp_pixel_type pixel_type, float gamma)
{
	GP_CHECK_VALID_PIXELTYPE(pixel_type);
	int channels = gp_pixel_types[pixel_type].numchannels, i;

	GP_DEBUG(1, "Acquiring Gamma table %s gamma %f", gp_pixel_type_name(pixel_type), gamma);

	gp_gamma *res = malloc(sizeof(struct gp_gamma) + 2 * channels * sizeof(void*));

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
		unsigned int chan_size = gp_pixel_types[pixel_type].channels[i].size;
		res->tables[i] = get_table(gamma, chan_size, chan_size + 2);

		if (res->tables[i] == NULL) {
			gp_gamma_release(res);
			return NULL;
		}
	}

	/* And reverse tables, n + 2 bits -> n bits */
	for (i = 0; i < channels; i++) {
		unsigned int chan_size = gp_pixel_types[pixel_type].channels[i].size;
		res->tables[i + channels] = get_table(1/gamma, chan_size + 2, chan_size);

		if (res->tables[i] == NULL) {
			gp_gamma_release(res);
			return NULL;
		}
	}

	return res;
}

gp_gamma *gp_gamma_copy(gp_gamma *self)
{
	self->ref_count++;
	return self;
}

void gp_gamma_release(gp_gamma *self)
{
	int channels, i;

	if (!self)
		return;

	channels = gp_pixel_types[self->pixel_type].numchannels;

	GP_DEBUG(1, "Releasing Gamma table %s gamma %f", gp_pixel_type_name(self->pixel_type), self->tables[0]->gamma);

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

static const char *correction_type_name(enum gp_correction_type type)
{
	if (type > GP_CORRECTION_sRGB)
		return "Invalid";

	return correction_type_names[type];
}

void gp_gamma_print(const gp_gamma *self)
{
	printf("Correction tables:\n");

	const gp_pixel_type_desc *desc = gp_pixel_desc(self->pixel_type);

	unsigned int i;

	for (i = 0; i < desc->numchannels; i++) {
		enum gp_correction_type type = self->tables[i]->type;

		printf(" %s: %s", desc->channels[i].name,
		       correction_type_name(type));

		if (type == GP_CORRECTION_GAMMA)
			printf(" gamma = %.2f", self->tables[i]->gamma);

		printf("\n");
	}
}
