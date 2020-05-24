// SPDX-License-Identifier: GPL-2.1-or-later
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

int gp_match_webp(const void *buf)
{
	if (memcmp(buf, "RIFF", 4))
		return 0;

	if (memcmp(buf + 8, "WEBP", 4))
		return 0;

	return 1;
}

#ifdef HAVE_WEBP

#include <webp/decode.h>

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

		WebPIDecGetRGB(idec, &ly, NULL, NULL, NULL);
		if (gp_progress_cb_report(callback, ly, out->h, out->w)) {
			GP_DEBUG(1, "Operation aborted");
			err = ECANCELED;
			goto err2;
		}

	} while ((ret = gp_io_read(io, buf, sizeof(buf))));

	WebPIDelete(idec);
	WebPFreeDecBuffer(&config.output);

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

#else

int gp_read_webp_ex(gp_io GP_UNUSED(*io), gp_pixmap GP_UNUSED(**img),
                   gp_storage GP_UNUSED(*storage),
                   gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_WEBP */

const gp_loader gp_webp = {
#ifdef HAVE_WEBP
	.Read = gp_read_webp_ex,
#endif /* HAVE_WEBP */
	.Match = gp_match_webp,

	.fmt_name = "webp",
	.extensions = {"webp", NULL},
};
