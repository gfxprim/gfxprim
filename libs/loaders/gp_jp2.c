// SPDX-License-Identifier: LGPL-2.1-or-later
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
#include <core/gp_clamp.h>
#include <core/gp_get_put_pixel.h>

#include <loaders/gp_loaders.gen.h>

#define JP2_SIG "\x00\x00\x00\x0cjP\x20\x20\x0d\x0a\x87\x0a"
#define JP2_SIG_LEN 12

/* A raw codestream starts with SOC followed by SIZ */
#define J2K_SIG "\xff\x4f\xff\x51"
#define J2K_SIG_LEN 4

int gp_match_jp2(const void *buf)
{
	return !memcmp(buf, JP2_SIG, JP2_SIG_LEN) || !memcmp(buf, J2K_SIG, J2K_SIG_LEN);
}

#ifdef HAVE_OPENJPEG

#include <openjpeg.h>

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
	case OPJ_CLRSPC_EYCC:
		return "e-YCC";
	case OPJ_CLRSPC_CMYK:
		return "CMYK";
	default:
		return "Invalid";
	}
}

/* The I/O and the offset of the image data in it, seek is absolute */
struct jp2_io {
	gp_io *io;
	off_t start;
};

static OPJ_SIZE_T jp2_io_read(void *buf, OPJ_SIZE_T size, void *priv)
{
	struct jp2_io *jio = priv;
	ssize_t ret;
	ret = gp_io_read(jio->io, buf, size);

	if (ret <= 0)
		return (OPJ_SIZE_T)-1;

	return ret;
}

static OPJ_OFF_T jp2_io_skip(OPJ_OFF_T off, void *priv)
{
	struct jp2_io *jio = priv;

	if (gp_io_seek(jio->io, off, GP_SEEK_CUR) == (off_t)-1)
		return -1;

	return off;
}

static OPJ_BOOL jp2_io_seek(OPJ_OFF_T off, void *priv)
{
	struct jp2_io *jio = priv;

	return gp_io_seek(jio->io, jio->start + off, GP_SEEK_SET) != (off_t)-1;
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

/*
 * Converts row y of component i into w 8 bit samples, written step bytes
 * apart, i.e. directly into the component bytes of the pixmap row. The pixmap
 * has the size of the first component, components subsampled relative to it,
 * e.g. chroma, are upsampled.
 */
static inline void decode_row(const opj_image_t *img, unsigned int i,
                              gp_size y, gp_size w, uint8_t *out,
                              unsigned int step)
{
	const opj_image_comp_t *base = &img->comps[0];
	const opj_image_comp_t *comp = &img->comps[i];
	int sub = comp->dx != base->dx || comp->w < w;
	int add = comp->sgnd ? 1 << (comp->prec - 1) : 0;
	int shift = comp->prec >= 8 ? (int)comp->prec - 8 : -1;
	int max = comp->prec < 8 ? (1 << comp->prec) - 1 : 0;
	unsigned int cy = y;
	const OPJ_INT32 *row;
	gp_size x;

	if (comp->dy != base->dy)
		cy = (uint64_t)y * base->dy / comp->dy;

	row = comp->data + (size_t)GP_MIN(cy, comp->h - 1) * comp->w;

	for (x = 0; x < w; x++, out += step) {
		unsigned int cx = x;
		int v;

		if (sub)
			cx = GP_MIN((uint64_t)x * base->dx / comp->dx, comp->w - 1);

		v = row[cx] + add;

		if (shift >= 0)
			*out = GP_CLAMP(v >> shift, 0, 255);
		else
			*out = GP_CLAMP(v, 0, max) * 255 / max;
	}
}

/*
 * Full resolution 8 bit unsigned component, the common case, the samples can
 * be copied as they are.
 */
static int comp_is_plain(const opj_image_t *img, unsigned int i,
                         const gp_pixmap *res)
{
	const opj_image_comp_t *comp = &img->comps[i];

	return comp->prec == 8 && !comp->sgnd &&
	       comp->w == res->w && comp->h >= res->h;
}

/*
 * The row holds Y, Cb, Cr, converted in place to RGB888, i.e. B, G, R, with
 * ITU-R BT.601 coefficients in 16.16 fixed point, rounded.
 */
static void sycc_to_rgb(uint8_t *row, gp_size w)
{
	gp_size x;

	for (x = 0; x < w; x++, row += 3) {
		int l = row[0], u = row[1] - 128, v = row[2] - 128;

		row[0] = GP_CLAMP(l + ((116130 * u + 32768) >> 16), 0, 255);
		row[1] = GP_CLAMP(l - ((22554 * u + 46802 * v + 32768) >> 16), 0, 255);
		row[2] = GP_CLAMP(l + ((91881 * v + 32768) >> 16), 0, 255);
	}
}

/* How the components are stored into the pixmap */
enum jp2_layout {
	JP2_GRAY,
	JP2_RGB,
	JP2_SYCC,
	JP2_CMYK,
};

static int load_pixels(const opj_image_t *img, gp_pixmap *res,
                       enum jp2_layout layout, gp_progress_cb *callback)
{
	int plain = comp_is_plain(img, 0, res);
	gp_size x, y, w = res->w;
	unsigned int i;

	if (layout == JP2_RGB)
		plain = plain && comp_is_plain(img, 1, res) && comp_is_plain(img, 2, res);

	for (y = 0; y < res->h; y++) {
		uint8_t *row = GP_PIXEL_ADDR(res, 0, y);
		size_t off = (size_t)y * w;

		switch (layout) {
		case JP2_GRAY:
			if (plain) {
				const OPJ_INT32 *v = img->comps[0].data + off;

				for (x = 0; x < w; x++)
					row[x] = v[x];
			} else {
				decode_row(img, 0, y, w, row, 1);
			}
		break;
		/* RGB888 is stored as B, G, R in memory */
		case JP2_RGB:
			if (plain) {
				const OPJ_INT32 *r = img->comps[0].data + off;
				const OPJ_INT32 *g = img->comps[1].data + off;
				const OPJ_INT32 *b = img->comps[2].data + off;

				for (x = 0; x < w; x++) {
					*row++ = b[x];
					*row++ = g[x];
					*row++ = r[x];
				}
			} else {
				decode_row(img, 0, y, w, row + 2, 3);
				decode_row(img, 1, y, w, row + 1, 3);
				decode_row(img, 2, y, w, row, 3);
			}
		break;
		case JP2_SYCC:
			for (i = 0; i < 3; i++)
				decode_row(img, i, y, w, row + i, 3);

			sycc_to_rgb(row, w);
		break;
		/* CMYK8888 is stored as C, M, Y, K in memory */
		case JP2_CMYK:
			for (i = 0; i < 4; i++)
				decode_row(img, i, y, w, row + i, 4);
		break;
		}

		if (gp_progress_cb_report(callback, y, res->h, res->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

/*
 * Picks a pixel type and a layout for the color space, extra components, e.g.
 * alpha, are ignored.
 */
static gp_pixel_type pixel_type(const opj_image_t *img, enum jp2_layout *layout)
{
	switch (img->color_space) {
	case OPJ_CLRSPC_GRAY:
		*layout = JP2_GRAY;
		return img->numcomps >= 1 ? GP_PIXEL_G8 : GP_PIXEL_UNKNOWN;
	case OPJ_CLRSPC_SRGB:
		*layout = JP2_RGB;
		return img->numcomps >= 3 ? GP_PIXEL_RGB888 : GP_PIXEL_UNKNOWN;
	case OPJ_CLRSPC_SYCC:
		*layout = JP2_SYCC;
		return img->numcomps >= 3 ? GP_PIXEL_RGB888 : GP_PIXEL_UNKNOWN;
	case OPJ_CLRSPC_CMYK:
		*layout = JP2_CMYK;
		return img->numcomps >= 4 ? GP_PIXEL_CMYK8888 : GP_PIXEL_UNKNOWN;
	case OPJ_CLRSPC_UNSPECIFIED:
	case OPJ_CLRSPC_UNKNOWN:
		/* Guess by the number of components, 2 is gray with alpha */
		if (img->numcomps == 1 || img->numcomps == 2) {
			*layout = JP2_GRAY;
			return GP_PIXEL_G8;
		}

		if (img->numcomps >= 3) {
			*layout = JP2_RGB;
			return GP_PIXEL_RGB888;
		}

		return GP_PIXEL_UNKNOWN;
	default:
		return GP_PIXEL_UNKNOWN;
	}
}

int gp_read_jp2_ex(gp_io *io, gp_pixmap **rimg, gp_image_info *image_info,
                   gp_progress_cb *callback)
{
	opj_dparameters_t params;
	opj_codec_t *codec;
	opj_stream_t *stream;
	opj_image_t *img = NULL;
	struct jp2_io jio = {.io = io};
	uint8_t sig[J2K_SIG_LEN];
	gp_pixel_type ptype;
	enum jp2_layout layout = JP2_GRAY;
	gp_pixmap *res = NULL;
	off_t size;
	unsigned int i;
	int err = 0, ret = 1;

	gp_image_info_clear(image_info);

	/* A raw codestream (J2K) or the JP2 file format */
	if (gp_io_peek(io, sig, sizeof(sig)) == (off_t)-1) {
		GP_DEBUG(1, "Failed to read the signature");
		err = EIO;
		goto err0;
	}

	jio.start = gp_io_tell(io);

	opj_set_default_decoder_parameters(&params);

	if (!memcmp(sig, J2K_SIG, J2K_SIG_LEN))
		codec = opj_create_decompress(OPJ_CODEC_J2K);
	else
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

	stream = opj_stream_create(OPJ_J2K_STREAM_CHUNK_SIZE, OPJ_TRUE);

	if (!stream) {
		GP_DEBUG(1, "opj_stream_create failed");
		err = ENOMEM;
		goto err1;
	}

	opj_stream_set_read_function(stream, jp2_io_read);
	opj_stream_set_skip_function(stream, jp2_io_skip);
	opj_stream_set_seek_function(stream, jp2_io_seek);
	opj_stream_set_user_data(stream, &jio, NULL);

	/* The decoder needs the length to find the end of the boxes */
	size = gp_io_size(io);
	if (size != (off_t)-1 && size >= jio.start)
		opj_stream_set_user_data_length(stream, size - jio.start);

	if (!opj_read_header(stream, codec, &img)) {
		GP_DEBUG(1, "opj_read_header failed");
		err = EINVAL;
		goto err2;
	}

	GP_DEBUG(1, "Have image %ux%u-%ux%u colorspace=%s numcomps=%u",
	         img->x0, img->y0, img->x1, img->y1,
	         color_space_name(img->color_space), img->numcomps);

	if (!img->numcomps) {
		GP_DEBUG(1, "No image components");
		err = EINVAL;
		goto err3;
	}

	ptype = pixel_type(img, &layout);

	/* The reference grid is larger when all components are subsampled */
	gp_image_info_fill(image_info, img->comps[0].w, img->comps[0].h, ptype);
	fill_metadata(img, gp_image_info_meta_data(image_info));

	if (!rimg) {
		ret = 0;
		goto err3;
	}

	gp_progress_cb_report(callback, 0, 100, 100);

	if (!opj_decode(codec, stream, img) || !opj_end_decompress(codec, stream)) {
		GP_DEBUG(1, "opj_decode failed");
		err = EINVAL;
		goto err3;
	}

	/* The decoder expands palette images, the components may change */
	ptype = pixel_type(img, &layout);

	if (ptype == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unsupported colorspace %s with %u components",
		         color_space_name(img->color_space), img->numcomps);
		err = ENOSYS;
		goto err3;
	}

	for (i = 0; i < img->numcomps; i++) {
		opj_image_comp_t *comp = &img->comps[i];

		GP_DEBUG(2, "Component %u %ux%u dx=%u dy=%u prec=%u sgnd=%u",
		         i, comp->w, comp->h, comp->dx, comp->dy, comp->prec, comp->sgnd);

		if (!comp->data || !comp->w || !comp->h || !comp->dx || !comp->dy ||
		    !comp->prec || comp->prec > 31) {
			GP_DEBUG(1, "Invalid component %u", i);
			err = EINVAL;
			goto err3;
		}
	}

	res = gp_pixmap_alloc(img->comps[0].w, img->comps[0].h, ptype);

	if (!res) {
		GP_DEBUG(1, "Malloc failed :(");
		err = ENOMEM;
		goto err3;
	}

	if (ptype == GP_PIXEL_RGB888)
		gp_pixmap_srgb_set(res);

	err = load_pixels(img, res, layout, callback);
	if (err) {
		gp_pixmap_free(res);
		goto err3;
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
                   gp_image_info GP_UNUSED(*image_info),
                   gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_OPENJPEG */

const gp_loader gp_jp2 = {
#ifdef HAVE_OPENJPEG
	.read = gp_read_jp2_ex,
#endif /* HAVE_OPENJPEG */
	.match = gp_match_jp2,

	.fmt_name = "JPEG 2000",
	.extensions = {"jp2", "jpx", "j2k", "j2c", NULL},
};
