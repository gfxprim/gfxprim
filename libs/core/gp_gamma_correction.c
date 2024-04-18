// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

#include <math.h>

#include <core/gp_pixel.h>
#include <core/gp_debug.h>
#include <core/gp_gamma_correction.h>

static gp_gamma_table *tables = NULL;

static void fill_gamma_table8(gp_gamma_table *table, float gamma,
                              uint8_t in_bits, uint8_t out_bits)
{
	unsigned int i;
	unsigned int in_max = (1<<in_bits) - 1;
	unsigned int out_max = (1<<out_bits) - 1;

	GP_DEBUG(3, "Initalizing gamma table %f", gamma);

	for (i = 0; i < (1U<<in_bits); i++)
		table->u8[i] = pow((float)i / in_max, gamma) * out_max + 0.5;
}

static void fill_gamma_table16(gp_gamma_table *table, float gamma,
                               uint8_t in_bits, uint8_t out_bits)
{
	unsigned int i;
	unsigned int in_max = (1<<in_bits) - 1;
	unsigned int out_max = (1<<out_bits) - 1;

	GP_DEBUG(3, "Initalizing gamma table %f", gamma);

	for (i = 0; i < (1U<<in_bits); i++)
		table->u16[i] = pow((float)i / in_max, gamma) * out_max + 0.5;
}

static void fill_lin_to_srgb_table8(gp_gamma_table *table,
                                    uint8_t in_bits, uint8_t out_bits)
{
	unsigned int i;
	unsigned int in_max = (1<<in_bits) - 1;
	unsigned int out_max = (1<<out_bits) - 1;

	GP_DEBUG(3, "Initalizing linear %i to sRGB %i table", in_bits, out_bits);

	for (i = 0; i < (1U<<in_bits); i++) {
		float v = 1.00 * i / in_max;

		if (v <= 0.0031308)
			table->u8[i] = (12.92 * v) * out_max + 0.5;
		else
			table->u8[i] = (1.055 * pow(v, 1/2.4) - 0.055) * out_max + 0.5;
	}
}

static void fill_srgb_to_lin_table16(gp_gamma_table *table,
                                     uint8_t in_bits, uint8_t out_bits)
{
	unsigned int i;
	unsigned int in_max = (1<<in_bits) - 1;
	unsigned int out_max = (1<<out_bits) - 1;

	GP_DEBUG(3, "Initalizing sRGB %i to linear %i table", in_bits, out_bits);

	for (i = 0; i < (1U<<in_bits); i++) {
		float v = 1.00 * i / in_max;

		if (v <= 0.04045)
			table->u16[i] = (v / 12.92) * out_max + 0.5;
		else
			table->u16[i] = pow((v + 0.055)/1.055, 2.4) * out_max + 0.5;
	}
}

static void fill_srgb_to_lin_table8(gp_gamma_table *table,
                                    uint8_t in_bits, uint8_t out_bits)
{
	unsigned int i;
	unsigned int in_max = (1<<in_bits) - 1;
	unsigned int out_max = (1<<out_bits) - 1;

	GP_DEBUG(3, "Initalizing sRGB %i to linear %i table", in_bits, out_bits);

	for (i = 0; i < (1U<<in_bits); i++) {
		float v = 1.00 * i / in_max;

		if (v <= 0.04045)
			table->u8[i] = (v / 12.92) * out_max + 0.5;
		else
			table->u8[i] = pow((v + 0.055)/1.055, 2.4) * out_max + 0.5;
	}
}

static gp_gamma_table *get_table(gp_correction_type corr_type, float gamma,
                                 uint8_t in_bits, uint8_t out_bits)
{
	gp_gamma_table *i;

	for (i = tables; i; i = i->next) {
		if (gamma == i->gamma && in_bits == i->in_bits &&
		    out_bits == i->out_bits && corr_type == i->corr_type)
			break;
	}

	if (i) {
		GP_DEBUG(2, "Found %s table %f, in_bits %u, out_bits %u, ref_count %u",
		         gp_correction_type_name(corr_type), i->gamma,
		         i->in_bits, i->out_bits, i->ref_count);
		i->ref_count++;
		return i;
	}

	GP_DEBUG(2, "Creating %s table %f, in_bits %u, out_bits %u",
	         gp_correction_type_name(corr_type), gamma, in_bits, out_bits);

	i = malloc(sizeof(gp_gamma_table) + (1U<<in_bits) * (out_bits > 8 ? 2 : 1));

	if (!i) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	i->gamma = gamma;
	i->in_bits = in_bits;
	i->out_bits = out_bits;
	i->ref_count = 1;
	i->corr_type = corr_type;

	switch (corr_type) {
	case GP_CORRECTION_TYPE_GAMMA:
		if (out_bits > 8)
			fill_gamma_table16(i, gamma, in_bits, out_bits);
		else
			fill_gamma_table8(i, gamma, in_bits, out_bits);
	break;
	case GP_CORRECTION_TYPE_SRGB:
		if (out_bits > in_bits) {
			if (out_bits > 8)
				fill_srgb_to_lin_table16(i, in_bits, out_bits);
			else
				fill_srgb_to_lin_table8(i, in_bits, out_bits);
		} else {
			GP_ASSERT(out_bits <= 8);
			fill_lin_to_srgb_table8(i, in_bits, out_bits);
		}
	break;
	}

	/* Insert it into link list */
	i->next = tables;
	tables = i;

	return i;
}

static void put_table(gp_gamma_table *table)
{
	if (!table)
		return;

	table->ref_count--;

	GP_DEBUG(4, "Decreasing refcount %u for %s table in_bits=%u out_bits=%u gamma=%f",
	         table->ref_count, gp_correction_type_name(table->corr_type),
	         table->in_bits, table->out_bits, table->gamma);

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

static void ref_table(gp_gamma_table *table)
{
	if (!table)
		return;

	table->ref_count++;

	GP_DEBUG(4, "Increasing refcount %u for %s table in_bits=%u out_bits=%u gamma=%f",
	         table->ref_count, gp_correction_type_name(table->corr_type),
	         table->in_bits, table->out_bits, table->gamma);
}

gp_gamma *gp_gamma_acquire(gp_pixel_type pixel_type,
                           gp_correction_type corr_type, float gamma)
{
	GP_CHECK_VALID_PIXELTYPE(pixel_type);
	int channels = gp_pixel_types[pixel_type].numchannels, i;
	float inv_gamma;

	switch (corr_type) {
	case GP_CORRECTION_TYPE_GAMMA:
		gamma = roundf(gamma * 1000) / 1000;
		inv_gamma = 1/gamma;
	break;
	case GP_CORRECTION_TYPE_SRGB:
		gamma = 0;
		inv_gamma = 0;
	break;
	default:
		GP_WARN("Invalid correction type %i", corr_type);
		return NULL;
	};


	GP_DEBUG(1, "Acquiring %s correction table for %s gamma %f",
		 gp_correction_type_name(corr_type),
	         gp_pixel_type_name(pixel_type), gamma);

	gp_gamma *res = malloc(sizeof(struct gp_gamma));

	if (!res) {
		GP_WARN("Malloc failed :(");
		return NULL;
	}

	for (i = 0; i < GP_PIXEL_CHANS_MAX; i++) {
		res->lin[i] = NULL;
		res->enc[i] = NULL;
	}

	res->pixel_type = pixel_type;
	res->ref_count = 1;

	for (i = 0; i < channels; i++) {
		unsigned int chan_size = gp_pixel_channel_bits(pixel_type, i);
		unsigned int chan_lin_size = gp_pixel_channel_lin_bits(pixel_type, i);

		if (chan_size == chan_lin_size)
			continue;

		res->lin[i] = get_table(corr_type, gamma, chan_size, chan_lin_size);
		res->enc[i] = get_table(corr_type, inv_gamma, chan_lin_size, chan_size);

		if (!res->lin[i] || !res->enc[i]) {
			gp_gamma_decref(res);
			return NULL;
		}
	}

	return res;
}

gp_gamma *gp_gamma_incref(gp_gamma *self)
{
	unsigned int i;

	if (!self)
		return NULL;

	GP_DEBUG(3, "Increasing refcount for %s table %s gamma %f",
	         gp_correction_type_name(self->lin[0]->corr_type),
	         gp_pixel_type_name(self->pixel_type), self->lin[0]->gamma);

	self->ref_count++;

	for (i = 0; i < GP_PIXEL_CHANS_MAX; i++) {
		ref_table(self->lin[i]);
		ref_table(self->enc[i]);
	}

	return self;
}

void gp_gamma_decref(gp_gamma *self)
{
	int i;

	if (!self)
		return;

	const char *correction_name = gp_correction_type_name(self->lin[0]->corr_type);
	const float gamma = self->lin[0]->gamma;

	GP_DEBUG(3, "Decreasing refcount for %s table %s gamma %f",
	         correction_name, gp_pixel_type_name(self->pixel_type), gamma);

	for (i = 0; i < GP_PIXEL_CHANS_MAX; i++) {
		put_table(self->lin[i]);
		put_table(self->enc[i]);
	}

	if (--self->ref_count == 0) {
		GP_DEBUG(2, "Freeing %s table %s gamma %f",
	                 correction_name, gp_pixel_type_name(self->pixel_type), gamma);
		free(self);
	}
}

static const char *correction_type_names[] = {
	"Gamma",
	"sRGB",
};

const char *gp_correction_type_name(enum gp_correction_type type)
{
	if (type > GP_CORRECTION_TYPE_SRGB)
		return "Invalid";

	return correction_type_names[type];
}

void gp_gamma_print(const gp_gamma *self)
{
	printf("Correction tables:\n");

	const gp_pixel_type_desc *desc = gp_pixel_desc(self->pixel_type);

	unsigned int i;

	for (i = 0; i < desc->numchannels; i++) {
		enum gp_correction_type type = self->lin[i]->corr_type;

		printf(" %s: %s", desc->channels[i].name,
		       gp_correction_type_name(type));

		if (type == GP_CORRECTION_TYPE_GAMMA)
			printf(" gamma = %.2f", self->lin[i]->gamma);

		printf("\n");
	}
}
