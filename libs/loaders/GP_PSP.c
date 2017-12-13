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
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  Paint Shop Pro Image loader.

  Written using documentation available freely on the internet.

 */

#include <stdint.h>
#include <inttypes.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <core/GP_Debug.h>
#include <core/GP_Pixel.h>
#include <core/GP_GetPutPixel.h>
#include <loaders/GP_Loaders.gen.h>

#define PSP_SIGNATURE "Paint Shop Pro Image File\n\x1a\0\0\0\0\0\0\0\0"
#define PSP_SIGNATURE_LEN 32

struct psp_version {
	uint16_t major;
	uint16_t minor;
};

int gp_match_psp(const void *buf)
{
	return !memcmp(buf, PSP_SIGNATURE, PSP_SIGNATURE_LEN);
}

enum psp_block_id {
	PSP_IMAGE_BLOCK,
	PSP_CREATOR_BLOCK,
	PSP_COLOR_BLOCK,
	PSP_LAYER_START_BLOCK,
	PSP_LAYER_BLOCK,
	PSP_CHANNEL_BLOCK,
	PSP_SELECTION_BLOCK,
	PSP_ALPHA_BANK_BLOCK,
	PSP_ALPHA_CHANNEL_BLOCK,
	PSP_COMPOSITE_IMAGE_BLOCK,
	PSP_EXTENDED_DATA_BLOCK,
	PSP_TUBE_BLOCK,
	PSP_ADJUSTMENT_EXTENSION_BLOCK,
	PSP_VECTOR_EXTENSION_BLOCK,
	PSP_SHAPE_BLOCK,
	PSP_PAINTSTYLE_BLOCK,
	PSP_COMPOSITE_IMAGE_BANK_BLOCK,
	PSP_COMPOSITE_ATTRIBUTES_BLOCK,
	PSP_JPEG_BLOCK,
};

static const char *psp_block_id_name(enum psp_block_id id)
{
	switch (id) {
	case PSP_IMAGE_BLOCK:
		return "Image";
	case PSP_CREATOR_BLOCK:
		return "Creator";
	case PSP_COLOR_BLOCK:
		return "Color";
	case PSP_LAYER_START_BLOCK:
		return "Layer Start";
	case PSP_LAYER_BLOCK:
		return "Layer";
	case PSP_CHANNEL_BLOCK:
		return "Channel";
	case PSP_SELECTION_BLOCK:
		return "Selection";
	case PSP_ALPHA_BANK_BLOCK:
		return "Alpha Bank";
	case PSP_ALPHA_CHANNEL_BLOCK:
		return "Alpha Channel";
	case PSP_COMPOSITE_IMAGE_BLOCK:
		return "Composite image";
	case PSP_EXTENDED_DATA_BLOCK:
		return "Extended Data";
	case PSP_TUBE_BLOCK:
		return "Tube";
	case PSP_ADJUSTMENT_EXTENSION_BLOCK:
		return "Adjustment Extension";
	case PSP_VECTOR_EXTENSION_BLOCK:
		return "Vector Extension";
	case PSP_SHAPE_BLOCK:
		return "Shape";
	case PSP_PAINTSTYLE_BLOCK:
		return "Paintstyle";
	case PSP_COMPOSITE_IMAGE_BANK_BLOCK:
		return "Composite Image Bank";
	case PSP_COMPOSITE_ATTRIBUTES_BLOCK:
		return "Composite Attributes";
	case PSP_JPEG_BLOCK:
		return "JPEG";
	default:
		return "Unknown";
	}
}

enum psp_comp_type {
	PSP_COMP_NONE,
	PSP_COMP_RLE,
	PSP_COMP_LZ77,
	PSP_COMP_JPEG,
};

static const char *psp_comp_type_name(enum psp_comp_type type)
{
	switch (type) {
	case PSP_COMP_NONE:
		return "None";
	case PSP_COMP_RLE:
		return "RLE";
	case PSP_COMP_LZ77:
		return "LZ77";
	case PSP_COMP_JPEG:
		return "JPEG";
	default:
		return "Unknown";
	}
}

struct psp_img_attrs {
	int32_t w;
	int32_t h;
	/* resolution */
	double res;
	uint8_t res_metric;
	/* compression, enum psp_comp_type */
	uint16_t comp_type;
	/* bit depth must be 1, 4, 8 or 24 */
	uint16_t bit_depth;
	/* plane count, must be 1 */
	uint16_t plane_count;
	/* color count = 2 ^ bit_depth */
	uint16_t color_count;
	/* grayscale flag, 0 -> not grayscale, 1 -> grayscale */
	uint8_t grayscale_flag;
	/* sum of the sizes of all layer color bitmaps */
	uint32_t total_img_size;
	/* active layer at the time of saving */
	uint32_t active_layer;
	uint16_t layer_count;

	/* internal loader flags */
	int is_loaded:1;
	uint8_t subblock;
	void *priv;
	gp_pixmap *img;
	gp_storage *storage;
};

static void fill_metadata(struct psp_img_attrs *attrs)
{
	gp_storage_add_int(attrs->storage, NULL, "Width", attrs->w);
	gp_storage_add_int(attrs->storage, NULL, "Height", attrs->h);
	gp_storage_add_string(attrs->storage, NULL, "Compression",
	                        psp_comp_type_name(attrs->comp_type));
	gp_storage_add_int(attrs->storage, NULL, "Bit Depth",
	                     attrs->bit_depth);
}

static int psp_read_general_img_attr_chunk(gp_io *io,
                                           struct psp_img_attrs *attrs)
{
	int err;

	if (attrs->is_loaded) {
		GP_WARN("Found Second Image Block");
		return EINVAL;
	}

	uint16_t general_image_info[] = {
		GP_IO_I4,   /* ??? */
		GP_IO_L4,   /* width */
		GP_IO_L4,   /* height */
		GP_IO_ARRAY | 8, /* resolution FIXME: double */
		GP_IO_BYTE, /* resolution metric */
		GP_IO_L2,   /* compression type */
		GP_IO_L2,   /* bit depth */
		GP_IO_L2,   /* plane count */
		GP_IO_L2,   /* color count */
		GP_IO_BYTE, /* grayscale flag */
		GP_IO_L4,   /* total image size */
		GP_IO_I2,   /* ??? */
		GP_IO_L4,   /* active layer */
		GP_IO_L2,   /* layer count */
		GP_IO_END
	};

	if (gp_io_readf(io, general_image_info, &attrs->w, &attrs->h,
	               &attrs->res, &attrs->res_metric, &attrs->comp_type,
		       &attrs->bit_depth, &attrs->plane_count,
		       &attrs->color_count, &attrs->grayscale_flag,
		       &attrs->total_img_size, &attrs->active_layer,
		       &attrs->layer_count) != 14) {
		err = errno;
		GP_DEBUG(1, "Failed to read Image attributes: %s",
		         strerror(errno));
		return err;
	}

	GP_DEBUG(3, "Image w=%u h=%u, compress=%s, bit_depth=%u, grayscale=%u",
	         attrs->w, attrs->h, psp_comp_type_name(attrs->comp_type),
		 attrs->bit_depth, attrs->grayscale_flag);

	GP_DEBUG(3, "Image colors=%u, layer_count=%u, active_layer=%u",
	         attrs->color_count, attrs->layer_count, attrs->active_layer);

	attrs->is_loaded = 1;

	if (attrs->storage)
		fill_metadata(attrs);

	return 0;
}

static int psp_next_block(gp_io *io, struct psp_img_attrs *attrs,
                          gp_progress_cb *callback);

static int psp_read_layer_start_block(gp_io *io, struct psp_img_attrs *attrs,
                                      gp_progress_cb *callback)
{
	int i;

	/* we are allready in subblock -> error */
	if (attrs->subblock) {
		GP_WARN("Layer Start block inside of Subblock");
		return EINVAL;
	}

	attrs->subblock++;

	for (i = 0; i < attrs->layer_count; i++)
		psp_next_block(io, attrs, callback);

	attrs->subblock--;

	return 0;
}

static int psp_read_composite_image_block(gp_io *io, struct psp_img_attrs *attrs,
                                          gp_progress_cb *callback)
{
	uint32_t i, composite_image_count;
	int err;

	/* we are allready in subblock -> error */
	if (attrs->subblock) {
		GP_WARN("Composite Image Bank block inside of a Subblock");
		return EINVAL;
	}

	uint16_t composite_image[] = {
		GP_IO_I4, /* chunk size */
		GP_IO_L4, /* composit image count */
		GP_IO_END,
	};

	if (gp_io_readf(io, composite_image, &composite_image_count) != 2) {
		err = errno;
		GP_DEBUG(1, "Failed to read Composite Image Bank Info Chunk");
		return err;
	}

	//TODO: utilize chunk_size

	GP_DEBUG(3, "Composite image count=%u", composite_image_count);

	attrs->subblock++;

	for (i = 0; i < composite_image_count; i++)
		psp_next_block(io, attrs, callback);

	attrs->subblock--;

	return 0;
}

enum psp_comp_img_type {
	PSP_IMAGE_COMPOSITE,
	PSP_IMAGE_THUMBNAIL,
};

static const char *psp_comp_img_type_name(enum psp_comp_img_type type)
{
	switch (type) {
	case PSP_IMAGE_COMPOSITE:
		return "Composite";
	case PSP_IMAGE_THUMBNAIL:
		return "Thumbnail";
	default:
		return "Unknown";
	}
}

struct psp_comp_img_attr_info {
	uint32_t w;
	uint32_t h;
	uint16_t bit_depth;
	uint16_t comp_type;
	uint16_t plane_count;
	uint32_t color_count;
	uint16_t comp_img_type;
};

static int psp_read_composite_attributes_block(gp_io *io, struct psp_img_attrs *attrs,
                                               gp_progress_cb *callback)
{
	struct psp_comp_img_attr_info info;
	int err;

	uint16_t info_chunk[] = {
		GP_IO_I4, /* chunk size */
		GP_IO_L4, /* width */
		GP_IO_L4, /* height */
		GP_IO_L2, /* bit depth */
		GP_IO_L2, /* compression type */
		GP_IO_L2, /* plane count */
		GP_IO_L4, /* color count */
		GP_IO_L2, /* composite image type */
		GP_IO_END
	};

	if (gp_io_readf(io, info_chunk, &info.w, &info.h, &info.bit_depth,
	               &info.comp_type, &info.plane_count, &info.color_count,
		       &info.comp_img_type) != 8) {
		err = errno;
		GP_DEBUG(1, "Failed to read Composite Image Attrs Info: %s",
		         strerror(err));
		return err;
	}

	GP_DEBUG(4, "Composite Image w=%u h=%u, bit_depth=%u, comp_type=%s, "
	            "comp_img_type=%s",
	             info.w, info.h, info.bit_depth, psp_comp_type_name(info.comp_type),
		     psp_comp_img_type_name(info.comp_img_type));

	attrs->priv = &info;
	attrs->subblock++;

	if (info.comp_img_type == PSP_IMAGE_COMPOSITE)
		psp_next_block(io, attrs, callback);

	attrs->subblock--;
	attrs->priv = NULL;

	return 0;
}

static int psp_read_jpeg(gp_io *io, struct psp_img_attrs *attrs,
                         gp_progress_cb *callback)
{
	int err;

	gp_io_seek(io, 14, GP_IO_SEEK_CUR);
/*
	if (fread(buf, sizeof(buf), 1, f) < 1) {
		GP_DEBUG(1, "Failed to read JPEG Information Chunk");
		return EIO;
	}
*/
	//TODO: utilize chunk_size

	GP_DEBUG(5, "JPEG Chunk");

	attrs->img = gp_read_jpg(io, callback);

	if (attrs->img == NULL) {
		err = errno;
		GP_WARN("Failed to load JPEG Data Chunk %s", strerror(err));
		return err;
	}

	return 0;
}

static int psp_next_block(gp_io *io, struct psp_img_attrs *attrs,
                          gp_progress_cb *callback)
{
	uint16_t block_id;
	uint32_t block_size;
	off_t offset;
	int err = 0;

	uint16_t block_header[] = {
		'~', 'B', 'K', 0x00,
		GP_IO_L2, /* block id */
		GP_IO_L4, /* block size */
		GP_IO_END
	};

	if (gp_io_readf(io, block_header, &block_id, &block_size) != 6) {
		err = errno;
		GP_DEBUG(1, "Failed to read block header: %s", strerror(errno));
		return err;
	}

	GP_DEBUG(2 + attrs->subblock, "%s Block size %u",
	         psp_block_id_name(block_id), block_size);

	offset = gp_io_tell(io) + block_size;

	switch (block_id) {
	case PSP_IMAGE_BLOCK:
		err = psp_read_general_img_attr_chunk(io, attrs);
	break;
	case PSP_LAYER_START_BLOCK:
		err = psp_read_layer_start_block(io, attrs, callback);
	break;
	case PSP_COMPOSITE_IMAGE_BANK_BLOCK:
		err = psp_read_composite_image_block(io, attrs, callback);
	break;
	case PSP_COMPOSITE_ATTRIBUTES_BLOCK:
		err = psp_read_composite_attributes_block(io, attrs, callback);
	break;
	case PSP_JPEG_BLOCK:
		err = psp_read_jpeg(io, attrs, callback);
	break;
	}

	if (err)
		return err;

	if (gp_io_seek(io, offset, GP_IO_SEEK_SET) != offset) {
		err = errno;
		GP_DEBUG(1, "Failed to seek to next block; %s",
		         strerror(errno));
		return err;
	}

	return 0;
}

int gp_read_psp_ex(gp_io *io, gp_pixmap **img, gp_storage *storage,
                   gp_progress_cb *callback)
{
	int err = 0;
	struct psp_img_attrs attrs = {.is_loaded = 0, .subblock = 0,
	                              .priv = NULL, .img = NULL, .storage = storage};
	struct psp_version version;

	uint16_t psp_header[] = {
		'P', 'a', 'i', 'n', 't', ' ',
		'S', 'h', 'o', 'p', ' ',
		'P', 'r', 'o', ' ',
		'I', 'm', 'a', 'g', 'e', ' ',
		'F', 'i', 'l', 'e', '\n', 0x1a,
		0x00, 0x00, 0x00, 0x00, 0x00,
		GP_IO_L2, /* version major */
		GP_IO_L2, /* version minor */
		GP_IO_END
	};

	if (gp_io_readf(io, psp_header, &version.major, &version.minor) != 34) {
		GP_DEBUG(1, "Failed to read file header");
		err = EIO;
		goto err0;
	}

	GP_DEBUG(1, "Have PSP image version %u.%u",
	         version.major, version.minor);

	if (storage) {
		gp_storage_add_int(storage, NULL, "Version Major", version.major);
		gp_storage_add_int(storage, NULL, "Version Minor", version.minor);
	}

	if (!img)
		return 0;

	while (!err) {
		err = psp_next_block(io, &attrs, callback);

		if (err)
			goto err0;

		if (attrs.img != NULL) {
			*img = attrs.img;
			return 0;
		}
	}

	errno = ENOSYS;
	return 1;
err0:
	errno = err;
	return 1;
}

const gp_loader gp_psp = {
	.Read = gp_read_psp_ex,
	.Match = gp_match_psp,
	.fmt_name = "Paint Shop Pro Image",
	.extensions = {"psp", "pspimage", NULL},
};
