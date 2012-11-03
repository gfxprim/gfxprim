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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
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

#include "core/GP_Debug.h"
#include "core/GP_Pixel.h"
#include "core/GP_GetPutPixel.h"
#include "GP_JPG.h"
#include "GP_PSP.h"

#define BUF_TO_8(buf, off) \
	(buf[off] + (buf[off+1]<<8) + (buf[off+2]<<16) + (buf[off+3]<<24) + \
	 ((uint64_t)buf[off+4]<<32) + ((uint64_t)buf[off+5]<<40) + \
	 ((uint64_t)buf[off+6]<<48) + ((uint64_t)buf[off+7]<<56))

#define BUF_TO_4(buf, off) \
	(buf[off] + (buf[off+1]<<8) + (buf[off+2]<<16) + (buf[off+3]<<24))

#define BUF_TO_2(buf, off) \
	(buf[off] + (buf[off+1]<<8))

#define PSP_SIGNATURE "Paint Shop Pro Image File\n\x1a\0\0\0\0\0\0\0\0"
#define PSP_SIGNATURE_LEN 32

struct psp_version {
	uint16_t major;
	uint16_t minor;
};

int GP_OpenPSP(const char *src_path, FILE **f)
{
	int err;

	*f = fopen(src_path, "rb");

	if (*f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s' : %s",
		            src_path, strerror(errno));
		goto err2;
	}

	char buf[36];

	if (fread(buf, sizeof(buf), 1, *f) < 1) {
		GP_DEBUG(1, "Failed to read file header");
		err = EIO;
		goto err1;
	}

	if (memcmp(buf, PSP_SIGNATURE, PSP_SIGNATURE_LEN)) { 
		GP_DEBUG(1, "Invalid signature, not a PSP image?");
		err = EINVAL;
		goto err1;
	}

	struct psp_version version;

	version.major = BUF_TO_2(buf, 32);
	version.minor = BUF_TO_2(buf, 34);

	GP_DEBUG(1, "Have PSP image version %u.%u",
	         version.major, version.minor);

	return 0;
err1:
	fclose(*f);
err2:
	errno = err;
	return 1;
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
	GP_Context *img;
};

static int psp_read_general_img_attr_chunk(FILE *f,
                                           struct psp_img_attrs *attrs)
{
	uint8_t buf[38];

	if (attrs->is_loaded) {
		GP_WARN("Found Second Image Block");
		return EINVAL;
	}

	//TODO SHIFT!!!
	if (fread(buf, 4, 1, f) < 1) {
		GP_DEBUG(1, "Failed to read Image block header");
		return EIO;
	}

	if (fread(buf, sizeof(buf), 1, f) < 1) {
		GP_DEBUG(1, "Failed to read Image chunk data");
		return EIO;
	}

	attrs->w = BUF_TO_4(buf, 0);
	attrs->h = BUF_TO_4(buf, 4);
	attrs->res = BUF_TO_8(buf, 8);
	attrs->res_metric = buf[16];
	attrs->comp_type = BUF_TO_2(buf, 17);
	attrs->bit_depth = BUF_TO_2(buf, 19);
	attrs->plane_count = BUF_TO_2(buf, 21);
	attrs->color_count = BUF_TO_2(buf, 23);
	attrs->grayscale_flag = buf[25];
	attrs->total_img_size = BUF_TO_4(buf, 26);
	attrs->active_layer = BUF_TO_4(buf, 32);
	attrs->layer_count = BUF_TO_2(buf, 36);

	GP_DEBUG(3, "Image w=%u h=%u, compress=%s, bit_depth=%u, grayscale=%u",
	         attrs->w, attrs->h, psp_comp_type_name(attrs->comp_type),
		 attrs->bit_depth, attrs->grayscale_flag);

	GP_DEBUG(3, "Image colors=%u, layer_count=%u, active_layer=%u",
	         attrs->color_count, attrs->layer_count, attrs->active_layer); 

	attrs->is_loaded = 1;

	return 0;
}

static int psp_next_block(FILE *f, struct psp_img_attrs *attrs,
                          GP_ProgressCallback *callback);

static int psp_read_layer_start_block(FILE *f, struct psp_img_attrs *attrs,
                                      GP_ProgressCallback *callback)
{
	int i;

	/* we are allready in subblock -> error */
	if (attrs->subblock) {
		GP_WARN("Layer Start block inside of Subblock");
		return EINVAL;
	}

	attrs->subblock++;

	for (i = 0; i < attrs->layer_count; i++)
		psp_next_block(f, attrs, callback);

	attrs->subblock--;

	return 0;
}

static int psp_read_composite_image_block(FILE *f, struct psp_img_attrs *attrs,
                                          GP_ProgressCallback *callback)
{
	uint8_t buf[8];
	uint32_t chunk_size;
	uint32_t i, composite_image_count;
	
	/* we are allready in subblock -> error */
	if (attrs->subblock) {
		GP_WARN("Composite Image Bank block inside of a Subblock");
		return EINVAL;
	}
	
	if (fread(buf, sizeof(buf), 1, f) < 1) {
		GP_DEBUG(1, "Failed to read Composite Image Bank Info Chunk");
		return EIO;
	}

	chunk_size = BUF_TO_4(buf, 0);
	composite_image_count = BUF_TO_4(buf, 4);

	//TODO: utilize chunk_size

	GP_DEBUG(3, "Composite image count=%u", composite_image_count);

	attrs->subblock++;

	for (i = 0; i < composite_image_count; i++)
		psp_next_block(f, attrs, callback);

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

static int psp_read_composite_attributes_block(FILE *f, struct psp_img_attrs *attrs,
                                               GP_ProgressCallback *callback)
{
	uint8_t buf[24];
	struct psp_comp_img_attr_info info;
	uint32_t chunk_size;

	if (fread(buf, sizeof(buf), 1, f) < 1) {
		GP_DEBUG(1, "Failed to read Composite Image Bank Info Chunk");
		return EIO;
	}

	chunk_size = BUF_TO_4(buf, 0);
	
	info.w = BUF_TO_4(buf, 4);
	info.h = BUF_TO_4(buf, 8);
	info.bit_depth = BUF_TO_2(buf, 12);
	info.comp_type = BUF_TO_2(buf, 14);
	info.plane_count = BUF_TO_2(buf, 16);
	info.color_count = BUF_TO_4(buf, 18);
	info.comp_img_type = BUF_TO_2(buf, 22);

	GP_DEBUG(4, "Composite Image w=%u h=%u, bit_depth=%u, comp_type=%s, "
	            "comp_img_type=%s",
	             info.w, info.h, info.bit_depth, psp_comp_type_name(info.comp_type),
		     psp_comp_img_type_name(info.comp_img_type));

	attrs->priv = &info;
	attrs->subblock++;

	if (info.comp_img_type == PSP_IMAGE_COMPOSITE)
		psp_next_block(f, attrs, callback);

	attrs->subblock--;
	attrs->priv = NULL;

	return 0;
}

static int psp_read_jpeg(FILE *f, struct psp_img_attrs *attrs,
                         GP_ProgressCallback *callback)
{
	uint8_t buf[14];	
	uint32_t chunk_size;
	int err;
	
	if (fread(buf, sizeof(buf), 1, f) < 1) {
		GP_DEBUG(1, "Failed to read JPEG Information Chunk");
		return EIO;
	}

	chunk_size = BUF_TO_4(buf, 0);
	
	//TODO: utilize chunk_size

	GP_DEBUG(5, "JPEG Chunk");
	
	attrs->img = GP_ReadJPG(f, callback);

	if (attrs->img == NULL) {
		err = errno;
		GP_WARN("Failed to load JPEG Data Chunk %s", strerror(err));
		return err;
	}

	return 0;
}

#define GEN_IMG_HEADER_ID "~BK"
#define GEN_IMG_HEADER_ID_LEN 4

static int psp_next_block(FILE *f, struct psp_img_attrs *attrs,
                          GP_ProgressCallback *callback)
{
	uint8_t buf[10];
	uint16_t block_id;
	uint32_t block_size;
	long offset;
	int err = 0;

	if (fread(buf, sizeof(buf), 1, f) < 1) {
		GP_DEBUG(1, "Failed to read block header");
		return EIO;
	}

	if (memcmp(buf, GEN_IMG_HEADER_ID, GEN_IMG_HEADER_ID_LEN)) {
		GP_DEBUG(1, "Invalid block header identifier");
		return EINVAL;
	}

	block_id = BUF_TO_2(buf, 4);
	block_size = BUF_TO_4(buf, 6);

	GP_DEBUG(2 + attrs->subblock, "%s Block size %u",
	         psp_block_id_name(block_id), block_size);

	offset = ftell(f) + block_size;

	switch (block_id) {
	case PSP_IMAGE_BLOCK:
		err = psp_read_general_img_attr_chunk(f, attrs);
	break;
	case PSP_LAYER_START_BLOCK:
		err = psp_read_layer_start_block(f, attrs, callback);
	break;
	case PSP_COMPOSITE_IMAGE_BANK_BLOCK:
		err = psp_read_composite_image_block(f, attrs, callback);
	break;
	case PSP_COMPOSITE_ATTRIBUTES_BLOCK:
		err = psp_read_composite_attributes_block(f, attrs, callback);
	break;
	case PSP_JPEG_BLOCK:
		err = psp_read_jpeg(f, attrs, callback);
	break;
	}

	if (err)
		return err;

	if (fseek(f, offset, SEEK_SET) == -1) {
		err = errno;
		GP_DEBUG(1, "Failed to seek to next block");
		return errno;
	}

	return 0;
}

GP_Context *GP_ReadPSP(FILE *f, GP_ProgressCallback *callback)
{
	int err = 0;
	struct psp_img_attrs attrs = {.is_loaded = 0, .subblock = 0,
	                              .priv = NULL, .img = NULL};

	while (!err) {
		err = psp_next_block(f, &attrs, callback);
	
		if (err)
			goto err1;
	
		if (attrs.img != NULL) {
			fclose(f);
			return attrs.img;
		}
	}

	fclose(f);
	errno = ENOSYS;
	return NULL;
err1:
	fclose(f);
	errno = err;
	return NULL;
}

GP_Context *GP_LoadPSP(const char *src_path, GP_ProgressCallback *callback)
{
	FILE *f;

	if (GP_OpenPSP(src_path, &f))
		return NULL;

	return GP_ReadPSP(f, callback);
}
