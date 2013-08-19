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

#ifdef HAVE_OPENJPEG

#include <openjpeg-2.0/openjpeg.h>

#define JP2_SIG "\x00\x00\x00\x0cjP\x20\x20\x0d\x0a\x87\x0a"
#define JP2_SIG_LEN 12

int GP_MatchJP2(const void *buf)
{
	return !memcmp(buf, JP2_SIG, JP2_SIG_LEN);
}

int GP_OpenJP2(const char *src_path, FILE **f)
{
	int err;

	*f = fopen(src_path, "rb");

	if (*f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s' : %s",
		            src_path, strerror(errno));
		errno = err;
		return 1;
	}

	//TODO: check signature and rewind the stream

	return 0;
}

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
	GP_ProgressCallback *callback = priv;

	GP_DEBUG(1, "openjpeg: %s", msg);

	GP_ProgressCallbackReport(callback, 100, 100, 100);
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

GP_Context *GP_ReadJP2(FILE *f, GP_ProgressCallback *callback)
{
	opj_dparameters_t params;
	opj_codec_t *codec;
	opj_stream_t *stream;
	opj_image_t *img;

	GP_Context *res = NULL;
	GP_PixelType pixel_type;
	unsigned int i, x, y;
	int err = 0;

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

	stream = opj_stream_create_default_file_stream(f, OPJ_TRUE);

	if (!stream) {
		GP_DEBUG(1, "opj_stream_create_default_file_stream faled");
		err = ENOMEM;
		goto err1;
	}

	if (!opj_read_header(stream, codec, &img)) {
		GP_DEBUG(1, "opj_read_header failed");
		err = EINVAL;
		goto err2;
	}

	GP_DEBUG(1, "Have image %ux%u-%ux%u colorspace=%s numcomps=%u",
	         img->x0, img->y0, img->x1, img->y1,
	         color_space_name(img->color_space), img->numcomps);

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
err3:
	opj_image_destroy(img);
err2:
	opj_stream_destroy(stream);
err1:
	opj_destroy_codec(codec);
err0:
	if (err)
		errno = err;
	return res;
}

GP_Context *GP_LoadJP2(const char *src_path, GP_ProgressCallback *callback)
{
	FILE *f;
	GP_Context *res;

	if (GP_OpenJP2(src_path, &f))
		return NULL;

	res = GP_ReadJP2(f, callback);

	fclose(f);

	return res;
}

#else

int GP_MatchJP2(const void GP_UNUSED(*buf))
{
	errno = ENOSYS;
	return -1;
}

int GP_OpenJP2(const char GP_UNUSED(*src_path), FILE GP_UNUSED(**f))
{
	errno = ENOSYS;
	return 1;
}

GP_Context *GP_ReadJP2(FILE GP_UNUSED(*f),
                      GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return NULL;
}

GP_Context *GP_LoadJP2(const char GP_UNUSED(*src_path),
                       GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return NULL;
}

#endif /* HAVE_OPENJPEG */
