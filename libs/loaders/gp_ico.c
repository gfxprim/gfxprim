// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

 ICO image support.

 */

#include <stdint.h>

#include <errno.h>
#include <string.h>

#include <core/gp_debug.h>
#include <loaders/gp_io.h>
#include <loaders/gp_io_parser.h>
#include <loaders/gp_loaders.gen.h>
#include <loaders/gp_bmp.h>

static const char ico_signature[] = {
	0x00,
	0x00,
	0x00,
	0x01,
};

int gp_match_ico(const void *buf)
{
	if (!memcmp(buf, ico_signature, sizeof(ico_signature)))
		return 1;

	return 0;
}

struct icondir_entry {
	uint8_t w;
	uint8_t h;
	uint8_t palette_colors;
	uint16_t color_planes;
	uint16_t bits_per_pixel;
	uint32_t data_size;
	uint32_t data_offset;
};

int gp_read_ico_ex(gp_io *io, gp_pixmap **img,
                   gp_storage *storage, gp_progress_cb *callback)
{
	gp_pixmap *res = NULL;
	uint16_t image_cnt;

	uint16_t icondir[] = {
		0x00, 0x00, 0x01, 0x00,
		GP_IO_L2,
		GP_IO_END,
	};

	if (gp_io_readf(io, icondir, &image_cnt) != 5) {
		GP_DEBUG(1, "Failed to read ICONDIR structure");
		return EIO;
	}

	GP_DEBUG(1, "Icon contains %u images", (unsigned int)image_cnt);

	if (image_cnt == 0) {
		GP_DEBUG(1, "No images found");
		return EIO;
	}

	uint16_t icondir_entry[] = {
		GP_IO_BYTE,
		GP_IO_BYTE,
		GP_IO_BYTE,
		GP_IO_IGN | 1,
		GP_IO_L2,
		GP_IO_L2,
		GP_IO_L4,
		GP_IO_L4,
		GP_IO_END,
	};

	struct icondir_entry ie;

	if (gp_io_readf(io, icondir_entry, &ie.w, &ie.h,
	                &ie.palette_colors, &ie.color_planes,
			&ie.bits_per_pixel, &ie.data_size,
			&ie.data_offset) != 8) {
		GP_DEBUG(1, "Failed to read ICONDIRENTRY structure");
		return EIO;
	}

	GP_DEBUG(1, "Have ICO image %ux%u, %u data_size, %u offset, "
		 "%u bpp, %u palette colors, %u color_planes",
		 ie.w, ie.h, ie.data_size, ie.data_offset,
		 ie.bits_per_pixel, ie.palette_colors, ie.color_planes);

	if (gp_io_seek(io, ie.data_offset, GP_SEEK_SET) == -1) {
		GP_DEBUG(1, "Failed to seek to image data");
		return EIO;
	}

	char buf[8];

	gp_io_peek(io, buf, sizeof(buf));

	if (gp_match_png(buf))
		return gp_read_png_ex(io, img, storage, callback);

	struct gp_bmp_info_header header;

	int ret = gp_bmp_read_info_header(io, &header);
	if (ret)
		return ret;

	gp_pixel_type ptype = gp_bmp_pixel_type(&header);

	/* Looks like a bug to bug compatibility with some windows */
	header.h /= 2;

	if (ie.bits_per_pixel)
		header.bpp = ie.bits_per_pixel;

	if (ie.palette_colors)
		header.palette_colors = ie.palette_colors;

	header.pixel_offset = ie.data_offset + header.header_size;

	/* Since there is no offset to pixel data we assume that it follows palette */
	switch (header.bpp) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 8:
		header.pixel_offset += gp_bmp_palette_size(&header);
	break;
	}

	res = gp_pixmap_alloc(header.w, header.h, ptype);
	if (!res) {
		errno = ENOMEM;
		return 1;
	}

	gp_pixmap_srgb_set(res);

	if (gp_bmp_read_pixels(io, &header, res, callback)) {
		gp_pixmap_free(res);
		return 1;
	}

	*img = res;

	return 0;
}

const gp_loader gp_ico = {
	.read = gp_read_ico_ex,
	.match = gp_match_ico,
	.fmt_name = "ICO",
	.extensions = {"ico", NULL},
};
