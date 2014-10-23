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

  JPEG 2000 image support using openjpeg library.

 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../../config.h"
#include "core/GP_Debug.h"
#include "core/GP_GetPutPixel.h"

#include "GP_JP2.h"

#define JP2_SIG "\x00\x00\x00\x0cjP\x20\x20\x0d\x0a\x87\x0a"
#define JP2_SIG_LEN 12

int GP_MatchJP2(const void *buf)
{
	return !memcmp(buf, JP2_SIG, JP2_SIG_LEN);
}

#ifdef HAVE_OPENJPEG

#include <openjpeg-2.0/openjpeg.h>

static void jp2_err_callback(const char *msg, void *priv)
{
	(void) priv;
	GP_WARN("openjpeg: %s", msg);
}

static void jp2_warn_callback(const char *msg, void *priv)
{
	(void) priv;
	GP_WARN("openjpeg: %s", msg);
}

static void jp2_info_callback(const char *msg, void *priv)
{
	(void) priv;
	GP_DEBUG(1, "openjpeg: %s", msg);
}

static const char *color_space_name(OPJ_COLOR_SPACE color_space)
{
	switch (color_space) {
	case OPJ_CLRSPC_UNKNOWN:
		return "Unknown";
	case OPJ_CLRSPC_UNSPECIFIED:
		return "Unspecified";
	case OPJ_CLRSPC_SRGB:
		return "sRGB";
	case OPJ_CLRSPC_GRAY:
		return "Grayscale";
	case OPJ_CLRSPC_SYCC:
		return "YUV";
	default:
		return "Invalid";
	}
}

static OPJ_SIZE_T jp2_io_read(void *buf, OPJ_SIZE_T size, void *io)
{
	ssize_t ret;
	ret = GP_IORead(io, buf, size);

	if (ret == 0)
		return -1;

	return ret;
}

static void fill_metadata(opj_image_t *img, GP_DataStorage *storage)
{
	unsigned int i;

	GP_DataStorageAddInt(storage, NULL, "Width", img->x1 - img->x0);
	GP_DataStorageAddInt(storage, NULL, "Height", img->y1 - img->y0);
	GP_DataStorageAddString(storage, NULL, "Color Space",
	                        color_space_name(img->color_space));
	GP_DataStorageAddInt(storage, NULL, "Samples per Pixel", img->numcomps);

	for (i = 0; i < img->numcomps; i++) {
		char buf[32];
		GP_DataNode *comp_node;

		snprintf(buf, sizeof(buf), "Channel %u", i);

		comp_node = GP_DataStorageAddDict(storage, NULL, buf);

		GP_DataStorageAddInt(storage, comp_node, "Width", img->comps[i].w);
		GP_DataStorageAddInt(storage, comp_node, "Height", img->comps[i].h);
		GP_DataStorageAddInt(storage, comp_node, "Bits per Sample", img->comps[i].prec);
	}
}

int GP_ReadJP2Ex(GP_IO *io, GP_Context **rimg, GP_DataStorage *storage,
                 GP_ProgressCallback *callback)
{
	opj_dparameters_t params;
	opj_codec_t *codec;
	opj_stream_t *stream;
	opj_image_t *img;

	GP_PixelType pixel_type;
	GP_Context *res = NULL;
	unsigned int i, x, y;
	int err = 0, ret = 1;

	opj_set_default_decoder_parameters(&params);

	codec = opj_create_decompress(OPJ_CODEC_JP2);

	if (!codec) {
		GP_DEBUG(1, "opj_create_decompress failed");
		err = ENOMEM;
		goto err0;
	}

	opj_set_error_handler(codec, jp2_err_callback, NULL);
	opj_set_warning_handler(codec, jp2_warn_callback, NULL);
	opj_set_info_handler(codec, jp2_info_callback, callback);

	if (!opj_setup_decoder(codec, &params)) {
		GP_DEBUG(1, "opj_setup_decoder failed");
		err = ENOMEM;
		goto err1;
	}

	stream = opj_stream_default_create(OPJ_TRUE);

	if (!stream) {
		GP_DEBUG(1, "opj_stream_create_default_file_stream faled");
		err = ENOMEM;
		goto err1;
	}

	//TODO: Do we need seek and skip?
	opj_stream_set_read_function(stream, jp2_io_read);
	opj_stream_set_user_data(stream, io);

	if (!opj_read_header(stream, codec, &img)) {
		GP_DEBUG(1, "opj_read_header failed");
		err = EINVAL;
		goto err2;
	}

	if (storage)
		fill_metadata(img, storage);

	GP_DEBUG(1, "Have image %ux%u-%ux%u colorspace=%s numcomps=%u",
	         img->x0, img->y0, img->x1, img->y1,
	         color_space_name(img->color_space), img->numcomps);

	if (!rimg)
		return 0;

	/*
	 * Try to match the image information into pixel type.
	 *
	 * Unfortunately the images I had have color_space set
	 * to unspecified yet they were RGB888.
	 */
	for (i = 0; i < img->numcomps; i++) {
		opj_image_comp_t *comp = &img->comps[i];

		GP_DEBUG(2, "Component %u %ux%u bpp=%u",
		         i, comp->w, comp->h, comp->prec);

		if (comp->w != img->comps[0].w ||
		    comp->h != img->comps[0].h) {
			GP_DEBUG(1, "Component %u has different size", 1);
			err = ENOSYS;
			goto err3;
		}

		if (comp->prec != 8) {
			GP_DEBUG(1, "Component %u has different bpp", 1);
			err = ENOSYS;
			goto err3;
		}
	}

	switch (img->color_space) {
	case OPJ_CLRSPC_UNSPECIFIED:
		if (img->numcomps != 3) {
			GP_DEBUG(1, "Unexpected number of components");
			err = ENOSYS;
			goto err3;
		}
		pixel_type = GP_PIXEL_RGB888;
	break;
	default:
		GP_DEBUG(1, "Unsupported colorspace");
		err = ENOSYS;
		goto err3;
	}

	GP_ProgressCallbackReport(callback, 0, 100, 100);

	if (!opj_decode(codec, stream, img)) {
		GP_DEBUG(1, "opj_decode failed");
		err = EINVAL;
		goto err3;
	}

	res = GP_ContextAlloc(img->comps[0].w, img->comps[0].h, pixel_type);

	if (!res) {
		GP_DEBUG(1, "Malloc failed :(");
		err = ENOMEM;
		goto err3;
	}

	for (y = 0; y < res->h; y++) {
		for (x = 0; x < res->w; x++) {
			i = y * res->w + x;

			GP_Pixel p = img->comps[0].data[i] << 16|
			             img->comps[1].data[i] << 8 |
			             img->comps[2].data[i];

			GP_PutPixel_Raw_24BPP(res, x, y, p);
		}
	}

	GP_ProgressCallbackDone(callback);
	*rimg = res;
	ret = 0;
err3:
	opj_image_destroy(img);
err2:
	opj_stream_destroy(stream);
err1:
	opj_destroy_codec(codec);
err0:
	if (err)
		errno = err;
	return ret;
}

#else

int GP_ReadJP2Ex(GP_IO GP_UNUSED(*io), GP_Context GP_UNUSED(**img),
                 GP_DataStorage GP_UNUSED(*storage),
                 GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_OPENJPEG */

GP_Context *GP_LoadJP2(const char *src_path, GP_ProgressCallback *callback)
{
	return GP_LoaderLoadImage(&GP_JP2, src_path, callback);
}

GP_Context *GP_ReadJP2(GP_IO *io, GP_ProgressCallback *callback)
{
	return GP_LoaderReadImage(&GP_JP2, io, callback);
}

int GP_LoadJP2Ex(const char *src_path, GP_Context **img,
                 GP_DataStorage *storage, GP_ProgressCallback *callback)
{
	return GP_LoaderLoadImageEx(&GP_JP2, src_path, img, storage, callback);
}

struct GP_Loader GP_JP2 = {
#ifdef HAVE_OPENJPEG
	.Read = GP_ReadJP2Ex,
#endif /* HAVE_OPENJPEG */
	.Match = GP_MatchJP2,

	.fmt_name = "JPEG 2000",
	.extensions = {"jp2", "jpx", NULL},
};
