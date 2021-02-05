// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*

  GIF image support using giflib.

 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>

#include "../../config.h"
#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.gen.h>
#include "core/gp_fill.h"
#include <core/gp_debug.h>

#include <loaders/gp_io.h>
#include <loaders/gp_loaders.gen.h>

#ifdef HAVE_GIFLIB

#include <gif_lib.h>

#define GIF_SIGNATURE1 "GIF87a"
#define GIF_SIGNATURE1_LEN 6

#define GIF_SIGNATURE2 "GIF89a"
#define GIF_SIGNATURE2_LEN 6

int gp_match_gif(const void *buf)
{
	if (!memcmp(buf, GIF_SIGNATURE1, GIF_SIGNATURE1_LEN))
		return 1;

	if (!memcmp(buf, GIF_SIGNATURE2, GIF_SIGNATURE2_LEN))
		return 1;

	return 0;
}

static int gif_input_func(GifFileType* gif, GifByteType* bytes, int size)
{
	gp_io *io = gif->UserData;

	return gp_io_read(io, bytes, size);
}

static const char *rec_type_name(GifRecordType rec_type)
{
	switch (rec_type) {
	case UNDEFINED_RECORD_TYPE:
		return "Undefined";
	case SCREEN_DESC_RECORD_TYPE:
		return "ScreenDesc";
	case IMAGE_DESC_RECORD_TYPE:
		return "ImageDesc";
	case EXTENSION_RECORD_TYPE:
		return "Extension";
	case TERMINATE_RECORD_TYPE:
		return "Terminate";
	default:
		return "Invalid";
	}
}

static const char *gif_err_name(int err)
{
	switch (err) {
	case E_GIF_ERR_OPEN_FAILED:
		return "E_GIF_ERR_OPEN_FAILED";
	case E_GIF_ERR_WRITE_FAILED:
		return "E_GIF_ERR_WRITE_FAILED";
	case E_GIF_ERR_HAS_SCRN_DSCR:
		return "E_GIF_ERR_HAS_SCRN_DSCR";
	case E_GIF_ERR_HAS_IMAG_DSCR:
		return "E_GIF_ERR_HAS_IMAG_DSCR";
	case E_GIF_ERR_NO_COLOR_MAP:
		return "E_GIF_ERR_NO_COLOR_MAP";
	case E_GIF_ERR_DATA_TOO_BIG:
		return "E_GIF_ERR_DATA_TOO_BIG";
	case E_GIF_ERR_NOT_ENOUGH_MEM:
		return "E_GIF_ERR_NOT_ENOUGH_MEM";
	case E_GIF_ERR_DISK_IS_FULL:
		return "E_GIF_ERR_DISK_IS_FULL";
	case E_GIF_ERR_CLOSE_FAILED:
		return "E_GIF_ERR_CLOSE_FAILED";
	case E_GIF_ERR_NOT_WRITEABLE:
		return "E_GIF_ERR_NOT_WRITEABLE";
	default:
		return "UNKNOWN";
	}
}

static int gif_err(GifFileType *gf)
{
#if defined(GIFLIB_MAJOR) && GIFLIB_MAJOR >= 5
	return gf->Error;
#else
	(void) gf;
	return GifLastError();
#endif
}

static int read_extensions(GifFileType *gf)
{
	uint8_t *gif_ext_ptr;
	int gif_ext_type;

	//TODO: Should we free them?

	if (DGifGetExtension(gf, &gif_ext_type, &gif_ext_ptr) != GIF_OK) {
		GP_DEBUG(1, "DGifGetExtension() error %s (%i)",
		         gif_err_name(gif_err(gf)), gif_err(gf));
		return EIO;
	}

	GP_DEBUG(2, "Have GIF extension type %i (ignoring)", gif_ext_type);

	do {
		if (DGifGetExtensionNext(gf, &gif_ext_ptr) != GIF_OK) {
			GP_DEBUG(1, "DGifGetExtension() error %s (%i)",
			         gif_err_name(gif_err(gf)), gif_err(gf));
			return EIO;
		}

	} while (gif_ext_ptr != NULL);

	return 0;
}

static inline GifColorType *get_color_from_map(ColorMapObject *map, int idx)
{
	if (map->ColorCount <= idx) {
		GP_DEBUG(1, "Invalid colormap index %i (%i max)",
		         map->ColorCount, idx);
		return map->Colors;
	}

	return &map->Colors[idx];
}

static inline gp_pixel get_color(GifFileType *gf, uint32_t idx)
{
	GifColorType *color;

	//TODO: no color map?
	if (gf->SColorMap == NULL)
		return 0;

	color = get_color_from_map(gf->SColorMap, idx);

	return  GP_PIXEL_CREATE_RGB888(color->Red, color->Green, color->Blue);
}

static int get_bg_color(GifFileType *gf, gp_pixel *pixel)
{
	GifColorType *color;

	if (gf->SColorMap == NULL)
		return 0;

	color = get_color_from_map(gf->SColorMap, gf->SBackGroundColor);

	*pixel = GP_PIXEL_CREATE_RGB888(color->Red, color->Green, color->Blue);

	return 1;
}

/*
 * The interlacing consists of 8 pixel high strips. Each pass adds some lines
 * into each strip. This function maps y in the gif buffer to real y.
 */
static inline unsigned int interlace_real_y(GifFileType *gf, unsigned int y)
{
	const unsigned int h = gf->Image.Height;
	unsigned int real_y;

	/* Pass 1: Line 0 for each strip */
	real_y = 8 * y;

	if (real_y < h)
		return real_y;

	/* Pass 2: Line 4 for each strip */
	real_y = 8 * (y - (h - 1)/8 - 1) + 4;

	if (real_y < h)
		return real_y;

	/* Pass 3: Lines 2 and 6 */
	real_y = 4 * (y - (h - 1)/4 - 1) + 2;

	if (real_y < h)
		return real_y;

	/* Pass 4: Lines 1, 3, 5, and 7 */
	real_y = 2 * (y - h/2 - h%2) + 1;

	if (real_y < h)
		return real_y;

	GP_BUG("real_y > h");

	return 0;
}

static void fill_metadata(GifFileType *gf, gp_storage *storage)
{
	gp_storage_add_int(storage, NULL, "Width", gf->SWidth);
	gp_storage_add_int(storage, NULL, "Height", gf->SHeight);
	gp_storage_add_int(storage, NULL, "Interlace", gf->Image.Interlace);
}

int gp_read_gif_ex(gp_io *io, gp_pixmap **img,
                   gp_storage *storage, gp_progress_cb *callback)
{
	GifFileType *gf;
	GifRecordType rec_type;
	gp_pixmap *res = NULL;
	gp_pixel bg;
	int32_t x, y;
	int err;

	errno = 0;
#if defined(GIFLIB_MAJOR) && GIFLIB_MAJOR >= 5
	gf = DGifOpen(io, gif_input_func, NULL);
#else
	gf = DGifOpen(io, gif_input_func);
#endif

	if (gf == NULL) {
		/*
		 * The giflib uses open() so when we got a failure and errno
		 * is set => open() has failed.
		 *
		 * When errno is not set the file content was not valid so we
		 * set errno to EIO.
		 */
		if (errno == 0)
			errno = EIO;

		return 1;
	}

	GP_DEBUG(1, "Have GIF image %ix%i, %i colors, %i bpp",
	         gf->SWidth, gf->SHeight, gf->SColorResolution,
		 gf->SColorMap ? gf->SColorMap->BitsPerPixel : -1);

	do {
		if (DGifGetRecordType(gf, &rec_type) != GIF_OK) {
			//TODO: error handling
			GP_DEBUG(1, "DGifGetRecordType() error %s (%i)",
			         gif_err_name(gif_err(gf)), gif_err(gf));
			err = EIO;
			goto err1;
		}

		GP_DEBUG(2, "Have GIF record type %s",
		         rec_type_name(rec_type));

		switch (rec_type) {
		case EXTENSION_RECORD_TYPE:
			if ((err = read_extensions(gf)))
				goto err1;
			continue;
		case IMAGE_DESC_RECORD_TYPE:
		break;
		default:
			continue;
		}

		if (DGifGetImageDesc(gf) != GIF_OK) {
			//TODO: error handling
			GP_DEBUG(1, "DGifGetImageDesc() error %s (%i)",
			         gif_err_name(gif_err(gf)), gif_err(gf)); 
			err = EIO;
			goto err1;
		}

		if (storage)
			fill_metadata(gf, storage);

		GP_DEBUG(1, "Have GIF Image left-top %ix%i, width-height %ix%i,"
		         " interlace %i, bpp %i", gf->Image.Left, gf->Image.Top,
			 gf->Image.Width, gf->Image.Height, gf->Image.Interlace,
			 gf->Image.ColorMap ? gf->Image.ColorMap->BitsPerPixel : -1);

		if (!img)
			break;

		res = gp_pixmap_alloc(gf->SWidth, gf->SHeight,
				      GP_PIXEL_RGB888);

		if (res == NULL) {
			err = ENOMEM;
			goto err1;
		}

		/* If background color is defined, use it */
		if (get_bg_color(gf, &bg)) {
			GP_DEBUG(1, "Filling bg color %x", bg);
			gp_fill(res, bg);
		}

		/* Now finally read gif image data */
		for (y = gf->Image.Top; y < gf->Image.Height; y++) {
			uint8_t line[gf->Image.Width];

			DGifGetLine(gf, line, gf->Image.Width);

			unsigned int real_y = y;

			if (gf->Image.Interlace) {
				real_y = interlace_real_y(gf, y);
				GP_DEBUG(3, "Interlace y -> real_y %u %u", y, real_y);
			}

			//TODO: just now we have only 8BPP
			for (x = 0; x < gf->Image.Width; x++)
				gp_putpixel_raw_24BPP(res, x + gf->Image.Left, real_y, get_color(gf, line[x]));

			if (gp_progress_cb_report(callback, y - gf->Image.Top,
			                              gf->Image.Height,
						      gf->Image.Width)) {
				GP_DEBUG(1, "Operation aborted");
				err = ECANCELED;
				goto err2;
			}
		}

		//TODO: now we exit after reading first image
		break;

	} while (rec_type != TERMINATE_RECORD_TYPE);

#if defined(GIFLIB_MAJOR) && GIFLIB_MAJOR >= 5 && GIFLIB_MINOR >= 1
	DGifCloseFile(gf, NULL);
#else
	DGifCloseFile(gf);
#endif

	/* No Image record found :( */
	if (img && !res) {
		errno = EINVAL;
		return 1;
	}

	if (img)
		*img = res;

	return 0;
err2:
	gp_pixmap_free(res);
err1:
#if defined(GIFLIB_MAJOR) && GIFLIB_MAJOR >= 5 && GIFLIB_MINOR >= 1
	DGifCloseFile(gf, NULL);
#else
	DGifCloseFile(gf);
#endif
	errno = err;
	return 1;
}

#else

int gp_match_gif(const void GP_UNUSED(*buf))
{
	errno = ENOSYS;
	return -1;
}

int gp_read_gif_ex(gp_io GP_UNUSED(*io), gp_pixmap GP_UNUSED(**img),
                   gp_storage GP_UNUSED(*storage), gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return -1;
}

#endif /* HAVE_GIFLIB */

const gp_loader gp_gif = {
#ifdef HAVE_GIFLIB
	.Read = gp_read_gif_ex,
#endif
	.Match = gp_match_gif,

	.fmt_name = "Graphics Interchange Format",
	.extensions = {"gif", NULL},
};
