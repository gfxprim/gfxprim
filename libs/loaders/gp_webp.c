// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2020 Cyril Hrubis <metan@ucw.cz>
 */

/*

  webp image support using libwepb library.

 */

#include <errno.h>
#include <string.h>

#include "../../config.h"
#include <core/gp_debug.h>
#include <core/gp_get_put_pixel.h>

#include <loaders/gp_io.h>
#include <loaders/gp_loaders.gen.h>
#include <loaders/gp_line_convert.h>

int gp_match_webp(const void *buf)
{
	if (memcmp(buf, "RIFF", 4))
		return 0;

	if (memcmp(buf + 8, "WEBP", 4))
		return 0;

	return 1;
}

static gp_pixel_type out_pixel_types[] = {
	GP_PIXEL_BGR888,
	GP_PIXEL_UNKNOWN
};

#ifdef HAVE_WEBP

#include <webp/decode.h>
#include <webp/encode.h>

static void fill_metadata(gp_storage *storage, int w, int h)
{
	gp_storage_add_int(storage, NULL, "Width", w);
	gp_storage_add_int(storage, NULL, "Height", h);
}

int gp_read_webp_ex(gp_io *io, gp_pixmap **img, gp_storage *storage,
                    gp_progress_cb *callback)
{
	unsigned char buf[1024];
	WebPBitstreamFeatures features;
	gp_pixel_type ptype;
	int err;

	ssize_t ret = gp_io_read(io, buf, sizeof(buf));

	if (ret <= 0) {
		GP_DEBUG(1, "initial read failed");
		errno = EINVAL;
		return 1;
	}

	if (WebPGetFeatures(buf, ret, &features) != VP8_STATUS_OK) {
		GP_DEBUG(1, "Failed to get webp features");
		errno = EINVAL;
		return 1;
	}

	GP_DEBUG(1, "Have webp image %ix%i has_alpha=%i",
		 features.width, features.height, features.has_alpha);

	WebPDecoderConfig config = {};
	if (!WebPInitDecoderConfig(&config)) {
		GP_DEBUG(1, "Failed to initialize decoder");
		errno = EINVAL;
		return 1;
	}

	if (storage)
		fill_metadata(storage, features.width, features.height);

	if (!img)
		return 0;

	if (features.has_alpha) {
		config.output.colorspace = MODE_BGRA;
		ptype = GP_PIXEL_RGBA8888;
	} else {
		config.output.colorspace = MODE_BGR;
		ptype = GP_PIXEL_RGB888;
	}

	gp_pixmap *out = gp_pixmap_alloc(features.width, features.height, ptype);
	if (!out) {
		GP_DEBUG(1, "malloc() failed :-(");
		err = ENOMEM;
		goto err0;
	}

	config.output.u.RGBA.rgba = out->pixels;
	config.output.u.RGBA.stride = out->bytes_per_row;
	config.output.u.RGBA.size = out->bytes_per_row * out->h;
	config.output.is_external_memory = 1;

	WebPIDecoder* idec = WebPINewDecoder(&config.output);
	if (!idec) {
		GP_DEBUG(1, "failed to allocate decoder");
		goto err1;
	}

	int ly=0;

	do {
		VP8StatusCode status = WebPIAppend(idec, buf, ret);

		if (status != VP8_STATUS_OK && status != VP8_STATUS_SUSPENDED)
		    break;

		int sy = ly;

		WebPIDecGetRGB(idec, &ly, NULL, NULL, NULL);
		if (gp_progress_cb_report(callback, ly, out->h, out->w)) {
			GP_DEBUG(1, "Operation aborted");
			err = ECANCELED;
			goto err2;
		}

		//TODO ugly!
		if (features.has_alpha) {
			gp_size x, y;
			for (y = sy; y < (gp_size)ly; y++) {
				uint8_t *buf = GP_PIXEL_ADDR(out, 0, y);

				for (x = 0; x < out->w; x++) {
					gp_size xo = 4 * x;
					GP_SWAP(buf[xo + 0], buf[xo + 3]);
					GP_SWAP(buf[xo + 1], buf[xo + 3]);
					GP_SWAP(buf[xo + 2], buf[xo + 3]);
				}
			}
		}
	} while ((ret = gp_io_read(io, buf, sizeof(buf))));

	WebPIDelete(idec);
	WebPFreeDecBuffer(&config.output);

	gp_pixmap_srgb_set(out);

	*img = out;

	gp_progress_cb_done(callback);
	return 0;
err2:
	WebPIDelete(idec);
err1:
	gp_pixmap_free(out);
err0:
	WebPFreeDecBuffer(&config.output);
	errno = err;
	return 1;
}

static int webp_writer_func(const uint8_t* data, size_t data_size, const WebPPicture *picture)
{
	gp_io *io = picture->custom_ptr;

	gp_io_write(io, data, data_size);

	return 1;
}

static int webp_progress_hook(int percents, const WebPPicture *picture)
{
	gp_progress_cb *callback = picture->user_data;

	if (callback) {
		callback->percentage = percents;
		return !callback->callback(callback);
	}

	return 1;
}

int gp_write_webp_ex(const gp_pixmap *src, gp_io *io, gp_progress_cb *callback)
{
	int err;
	gp_pixel_type out_pix;
	WebPPicture picture = {};
	WebPConfig config;

	GP_DEBUG(1, "Writing WebP Image to I/O (%p)", io);

	out_pix = gp_line_convertible(src->pixel_type, out_pixel_types);
	if (out_pix == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unsupported pixel type %s",
		         gp_pixel_type_name(src->pixel_type));
		errno = ENOSYS;
		return 1;
	}

	if (!WebPConfigInit(&config)) {
		GP_DEBUG(1, "Failed to initialize encoder");
		errno = EINVAL;
		return 1;
	}

	config.lossless = 1;

	if (!WebPPictureInit(&picture)) {
		GP_DEBUG(1, "Failed to initialize picture");
		errno = EINVAL;
		return 1;
	}

	picture.width = src->w;
	picture.height = src->h;
	picture.use_argb = 1;

	if (!WebPPictureAlloc(&picture)) {
		GP_DEBUG(1, "Malloc failed :(");
		errno = ENOMEM;
		return 1;
	}

	gp_line_convert convert = gp_line_convert_get(src->pixel_type, out_pix);
	uint32_t *dst_buf = picture.argb;
	gp_size x, y;

	picture.argb_stride = src->w;

	for (y = 0; y < src->h; y++) {
		uint8_t *src_buf = GP_PIXEL_ADDR(src, 0, y);
		uint8_t tmp[3 * src->w];

		if (convert) {
			convert(src_buf, tmp, src->w);
			src_buf = tmp;
		}

		/* WebPImage stores RGB pixels always with alpha channel. */
		for (x = 0; x < src->w; x++) {
			dst_buf[x] = (0xff000000) | (src_buf[3*x]<<16) | (src_buf[3*x+1]<<8) | src_buf[3*x+2];
		}
		dst_buf += src->w;
	}

	picture.writer = webp_writer_func;
	picture.custom_ptr = io;

	if (callback) {
		picture.progress_hook = webp_progress_hook;
		picture.user_data = callback;
	}

	err = WebPEncode(&config, &picture);

	switch (picture.error_code) {
	case VP8_ENC_OK:
	break;
	case VP8_ENC_ERROR_USER_ABORT:
		errno = ECANCELED;
	break;
	case VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY:
	case VP8_ENC_ERROR_OUT_OF_MEMORY:
		errno = ENOMEM;
	break;
	case VP8_ENC_ERROR_BAD_WRITE:
		errno = EIO;
	break;
	default:
		errno = EINVAL;
	}

	WebPPictureFree(&picture);

	return !err;
}

#else

int gp_read_webp_ex(gp_io GP_UNUSED(*io), gp_pixmap GP_UNUSED(**img),
                   gp_storage GP_UNUSED(*storage),
                   gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

int gp_write_webp_ex(const gp_pixmap GP_UNUSED(*src), gp_io GP_UNUSED(*io),
                     gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_WEBP */

const gp_loader gp_webp = {
#ifdef HAVE_WEBP
	.read = gp_read_webp_ex,
	.write = gp_write_webp_ex,
	.save_ptypes = out_pixel_types,
#endif /* HAVE_WEBP */
	.match = gp_match_webp,

	.fmt_name = "webp",
	.extensions = {"webp", NULL},
};
