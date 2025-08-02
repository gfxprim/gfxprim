// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

/*

  HEIF/HEIC image support.

 */

#include <errno.h>
#include <string.h>

#include "../../config.h"
#include <core/gp_debug.h>
#include <core/gp_get_put_pixel.h>

#include <loaders/gp_io.h>
#include <loaders/gp_icc.h>
#include <loaders/gp_exif.h>
#include <loaders/gp_loaders.gen.h>

int gp_match_heif(const void *buf)
{
	if (memcmp(buf+4, "ftyp", 4))
		return 0;

	if (!memcmp(buf+8, "heic", 4))
		return 1;

	if (!memcmp(buf+8, "heif", 4))
		return 1;

	if (!memcmp(buf+8, "avif", 4))
		return 1;

	return 0;
}

#ifdef HAVE_HEIF

#include <libheif/heif.h>

static void fill_metadata(gp_storage *storage, int w, int h)
{
	gp_storage_add_int(storage, NULL, "Width", w);
	gp_storage_add_int(storage, NULL, "Height", h);
}

static int64_t get_position_heif_reader(void *userdata)
{
	gp_io *io = userdata;

	return gp_io_tell(io);
}

static int read_heif_reader(void *data, size_t size, void *userdata)
{
	gp_io *io = userdata;

	return gp_io_fill(io, data, size);
}

static int seek_heif_reader(int64_t position, void *userdata)
{
	gp_io *io = userdata;

	return gp_io_seek(io, position, GP_SEEK_SET) == -1;
}

static enum heif_reader_grow_status wait_for_file_size_heif_reader(int64_t target_size, void *userdata)
{
	gp_io *io = userdata;
	off_t io_size;

	io_size = gp_io_size(io);

	if (io_size == -1)
		return heif_reader_grow_status_size_reached;

	if (io_size < target_size)
		return heif_reader_grow_status_size_beyond_eof;

	return heif_reader_grow_status_size_reached;
}

static struct heif_reader reader_heif_reader = {
	.reader_api_version = 1,
	.get_position = get_position_heif_reader,
	.read = read_heif_reader,
	.seek = seek_heif_reader,
	.wait_for_file_size = wait_for_file_size_heif_reader,
};

static void on_progress_heif(enum heif_progress_step step, int progress,
                      void *progress_user_data)
{
	gp_progress_cb *callback = progress_user_data;

	int max_progress = callback->percentage;

	(void) step;

	callback->percentage = 100.00 * progress / max_progress;
	callback->flags = callback->callback(callback);
	callback->percentage = max_progress;
}

static void start_progress_heif(enum heif_progress_step step, int max_progress,
                         void *progress_user_data)
{
	gp_progress_cb *callback = progress_user_data;

	(void) step;

	callback->percentage = max_progress;
	callback->flags = 0;
}

#if LIBHEIF_HAVE_VERSION(1, 19, 0)
static int cancel_decoding_heif(void *progress_user_data)
{
	gp_progress_cb *callback = progress_user_data;

	return callback->flags;
}
#endif /* 1.19.0 */

static void parse_exif_heif(struct heif_image_handle *image_handle,
                            gp_storage *storage, gp_correction_desc *corr_desc)
{
	struct heif_error err;
	heif_item_id item_id;
	uint8_t *exif;

	if (heif_image_handle_get_list_of_metadata_block_IDs(image_handle, "Exif", &item_id, 1) != 1)
		return;

	size_t exif_size = heif_image_handle_get_metadata_size(image_handle, item_id);

	if (exif_size <= 4)
		return;

	exif = malloc(exif_size);
	if (!exif) {
		GP_WARN("Malloc failed :(");
		return;
	}

	err = heif_image_handle_get_metadata(image_handle, item_id, exif);
	if (err.code) {
		GP_WARN("Failed to read exif");
		free(exif);
		return;
	}

	gp_io *mem_io = gp_io_mem(exif+4, exif_size-4, NULL);
	if (!mem_io) {
		free(exif);
		return;
	}

	gp_read_exif(mem_io, storage, corr_desc);

	gp_io_close(mem_io);
	free(exif);
}

#if LIBHEIF_HAVE_VERSION(1, 4, 0)
static void parse_color_profile_heif(struct heif_image_handle *image_handle,
                                     gp_storage *storage)
{
	switch (heif_image_handle_get_color_profile_type(image_handle)) {
	case heif_color_profile_type_not_present:
		GP_DEBUG(1, "No color profile");
		return;
	case heif_color_profile_type_nclx:
		GP_DEBUG(1, "NCLX color profile");
	break;
	case heif_color_profile_type_rICC:
		GP_DEBUG(1, "ICC color profile");
	break;
	case heif_color_profile_type_prof:
		GP_DEBUG(1, "PROF color profile");
	break;
	}

	uint8_t *icc;
	size_t icc_size;

	icc_size = heif_image_handle_get_raw_color_profile_size(image_handle);

	if (!icc_size) {
		GP_WARN("color profile size = 0");
		return;
	}

	icc = malloc(icc_size);
	if (!icc) {
		GP_WARN("Malloc failed :(");
		return;
	}

	heif_image_handle_get_raw_color_profile(image_handle, icc);

	gp_io *mem_io = gp_io_mem(icc, icc_size, NULL);
	if (!mem_io) {
		free(icc);
		return;
	}

	gp_read_icc(mem_io, storage);

	gp_io_close(mem_io);
	free(icc);
}
#endif /* 1.4.0 */

static inline int err_heif_to_errno(struct heif_error err)
{
	switch (err.code) {
	case heif_error_Invalid_input:
		return EINVAL;
	case heif_error_Unsupported_filetype:
	case heif_error_Unsupported_feature:
		return ENOSYS;
	case heif_error_Memory_allocation_error:
		return ENOMEM;
#if LIBHEIF_HAVE_VERSION(1, 19, 0)
	case heif_error_Canceled:
		return ECANCELED;
#endif /* 1.19.0 */
	default:
		return EINVAL;
	}
}

int gp_read_heif_ex(gp_io *io, gp_pixmap **img, gp_storage *storage,
                    gp_progress_cb *callback)
{
	struct heif_context *ctx;
	struct heif_error err;
	struct heif_image_handle *image_handle;
	int w, h, has_alpha;
	gp_correction_desc corr_desc = {.corr_type = -1};

	ctx = heif_context_alloc();

//	heif_context_set_max_decoding_threads(ctx, 0);

	err = heif_context_read_from_reader(ctx, &reader_heif_reader, io, NULL);
	if (err.code) {
		GP_WARN("Failed to read HEIF context %i", err.code);
		goto err0;
	}

	err = heif_context_get_primary_image_handle(ctx, &image_handle);
	if (err.code) {
		GP_WARN("Failed to get HEIF primary image %i", err.code);
		goto err0;
	}

	w = heif_image_handle_get_width(image_handle);
	h = heif_image_handle_get_height(image_handle);
	has_alpha = heif_image_handle_has_alpha_channel(image_handle);

	GP_DEBUG(1, "Have HEIF image %ix%i has_alpha=%i", w, h, has_alpha);

	fill_metadata(storage, w, h);
	parse_exif_heif(image_handle, storage, &corr_desc);
#if LIBHEIF_HAVE_VERSION(1, 4, 0)
	parse_color_profile_heif(image_handle, storage);
#endif /* 1.4.0 */

	if (img) {
		int stride;
		const uint8_t *pixels;
		struct heif_image *image;

		struct heif_decoding_options dec_opts = {
			.version = 6,
			.start_progress = start_progress_heif,
			.on_progress = on_progress_heif,
			.progress_user_data = callback,
#if LIBHEIF_HAVE_VERSION(1, 19, 0)
			.cancel_decoding = cancel_decoding_heif,
#endif /* 1.19.0 */
		};

		err = heif_decode_image(image_handle, &image,
				        heif_colorspace_RGB,
					heif_chroma_interleaved_RGB, callback ? &dec_opts : NULL);

		if (err.code) {
			GP_WARN("Failed to decode image %i", err.code);
			goto err1;
		}

		pixels = heif_image_get_plane_readonly(image, heif_channel_interleaved, &stride);

		*img = gp_pixmap_alloc_ex(w, h, GP_PIXEL_BGR888, stride);
		if (*img) {
			memcpy((*img)->pixels, pixels, h * stride);

			if (corr_desc.corr_type != -1)
				gp_pixmap_correction_set(*img, &corr_desc);
		}

		heif_image_release(image);

		if (!(*img)) {
			err.code = heif_error_Memory_allocation_error;
			goto err1;
		}
	}

	heif_image_handle_release(image_handle);
	heif_context_free(ctx);
	return 0;
err1:
	heif_image_handle_release(image_handle);
err0:
	heif_context_free(ctx);
	errno = err_heif_to_errno(err);
	return 1;
}

#else

int gp_read_heif_ex(gp_io GP_UNUSED(*io), gp_pixmap GP_UNUSED(**img),
                    gp_storage GP_UNUSED(*storage),
                    gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_HEIF */

const gp_loader gp_heif = {
#ifdef HAVE_HEIF
	.read = gp_read_heif_ex,
#endif /* HAVE_HEIF */
	.match = gp_match_heif,

	.fmt_name = "heif",
	.extensions = {"heic", "heif", "avif", NULL},
};
