// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*

  JPEG 2000 image support using openjpeg library.

 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "../../config.h"
#include <core/gp_debug.h>
#include <core/gp_get_put_pixel.h>

#include <loaders/gp_loaders.gen.h>

#define JP2_SIG "\x00\x00\x00\x0cjP\x20\x20\x0d\x0a\x87\x0a"
#define JP2_SIG_LEN 12

int gp_match_jp2(const void *buf)
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
	ret = gp_io_read(io, buf, size);

	if (ret == 0)
		return -1;

	return ret;
}

static void fill_metadata(opj_image_t *img, gp_storage *storage)
{
	unsigned int i;

	gp_storage_add_int(storage, NULL, "Width", img->x1 - img->x0);
	gp_storage_add_int(storage, NULL, "Height", img->y1 - img->y0);
	gp_storage_add_string(storage, NULL, "Color Space",
	                        color_space_name(img->color_space));
	gp_storage_add_int(storage, NULL, "Samples per Pixel", img->numcomps);

	for (i = 0; i < img->numcomps; i++) {
		char buf[32];
		gp_data_node *comp_node;

		snprintf(buf, sizeof(buf), "Channel %u", i);

		comp_node = gp_storage_add_dict(storage, NULL, buf);

		gp_storage_add_int(storage, comp_node, "Width", img->comps[i].w);
		gp_storage_add_int(storage, comp_node, "Height", img->comps[i].h);
		gp_storage_add_int(storage, comp_node, "Bits per Sample", img->comps[i].prec);
	}
}

int gp_read_jp2_ex(gp_io *io, gp_pixmap **rimg, gp_storage *storage,
                   gp_progress_cb *callback)
{
	opj_dparameters_t params;
	opj_codec_t *codec;
	opj_stream_t *stream;
	opj_image_t *img;

	gp_pixel_type pixel_type;
	gp_pixmap *res = NULL;
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

	gp_progress_cb_report(callback, 0, 100, 100);

	if (!opj_decode(codec, stream, img)) {
		GP_DEBUG(1, "opj_decode failed");
		err = EINVAL;
		goto err3;
	}

	res = gp_pixmap_alloc(img->comps[0].w, img->comps[0].h, pixel_type);

	if (!res) {
		GP_DEBUG(1, "Malloc failed :(");
		err = ENOMEM;
		goto err3;
	}

	for (y = 0; y < res->h; y++) {
		for (x = 0; x < res->w; x++) {
			i = y * res->w + x;

			gp_pixel p = img->comps[0].data[i] << 16|
			             img->comps[1].data[i] << 8 |
			             img->comps[2].data[i];

			gp_putpixel_raw_24BPP(res, x, y, p);
		}
	}

	gp_progress_cb_done(callback);
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

int gp_read_jp2_ex(gp_io GP_UNUSED(*io), gp_pixmap GP_UNUSED(**img),
                   gp_storage GP_UNUSED(*storage),
                   gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_OPENJPEG */

const gp_loader gp_jp2 = {
#ifdef HAVE_OPENJPEG
	.Read = gp_read_jp2_ex,
#endif /* HAVE_OPENJPEG */
	.Match = gp_match_jp2,

	.fmt_name = "JPEG 2000",
	.extensions = {"jp2", "jpx", NULL},
};
