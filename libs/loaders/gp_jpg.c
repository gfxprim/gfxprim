// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*

  JPG image support using jpeg library.

 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

#include "../../config.h"
#include <core/gp_debug.h>

#include <loaders/gp_exif.h>
#include <loaders/gp_icc.h>
#include <loaders/gp_line_convert.h>
#include <loaders/gp_loaders.gen.h>

/*
 * 0xff 0xd8 - start of image
 * 0xff 0x.. - start of frame
 */
#define JPEG_SIGNATURE "\xff\xd8\xff"
#define JPEG_SIGNATURE_LEN 3

int gp_match_jpg(const void *buf)
{
	return !memcmp(buf, JPEG_SIGNATURE, JPEG_SIGNATURE_LEN);
}

#ifdef HAVE_JPEG

#include <jpeglib.h>
#include <jerror.h>

struct my_jpg_err {
	struct jpeg_error_mgr error_mgr;
	jmp_buf setjmp_buf;
};

static void my_error_exit(j_common_ptr cinfo)
{
	struct my_jpg_err *my_err = (struct my_jpg_err*) cinfo->err;
	char msg[JMSG_LENGTH_MAX];

	cinfo->err->format_message(cinfo, msg);

	GP_WARN("jpeg lib error: %s", msg);

	longjmp(my_err->setjmp_buf, 1);
}

static const char *get_colorspace(J_COLOR_SPACE color_space)
{
	switch (color_space) {
	case JCS_GRAYSCALE:
		return "Grayscale";
	case JCS_RGB:
		return "RGB";
	case JCS_YCbCr:
		return "YCbCr";
	case JCS_CMYK:
		return "CMYK";
	case JCS_YCCK:
		return "YCCK";
	default:
		return "Unknown";
	};
}

static int load(struct jpeg_decompress_struct *cinfo, gp_pixmap *ret,
                gp_progress_cb *callback)
{
	while (cinfo->output_scanline < cinfo->output_height) {
		uint32_t y = cinfo->output_scanline;
                JSAMPROW addr = (void*)GP_PIXEL_ADDR(ret, 0, y);

		jpeg_read_scanlines(cinfo, &addr, 1);

		if (gp_progress_cb_report(callback, y, ret->h, ret->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static int load_cmyk(struct jpeg_decompress_struct *cinfo, gp_pixmap *ret,
                       gp_progress_cb *callback)
{
	while (cinfo->output_scanline < cinfo->output_height) {
		uint32_t y = cinfo->output_scanline;

		JSAMPROW addr = (void*)GP_PIXEL_ADDR(ret, 0, y);
		jpeg_read_scanlines(cinfo, &addr, 1);

		unsigned int i;
		uint8_t *buf = GP_PIXEL_ADDR(ret, 0, y);

		for (i = 0; i < ret->w; i++) {
			unsigned int j = 4 * i;

			buf[j]   = 0xff - buf[j];
			buf[j+1] = 0xff - buf[j+1];
			buf[j+2] = 0xff - buf[j+2];
			buf[j+3] = 0xff - buf[j+3];
		}

		if (gp_progress_cb_report(callback, y, ret->h, ret->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}


struct my_source_mgr {
	struct jpeg_source_mgr mgr;
	int start_of_file;
	void *buffer;
	size_t size;
	gp_io *io;
};

static void dummy_src(j_decompress_ptr GP_UNUSED(cinfo))
{
}

static boolean fill_input_buffer(struct jpeg_decompress_struct *cinfo)
{
	int ret;
	struct my_source_mgr* src = (void*)cinfo->src;

	ret = gp_io_read(src->io, src->buffer, src->size);

	if (ret <= 0) {
		GP_WARN("Failed to fill buffer, IORead returned %i", ret);
		if (src->start_of_file)
			ERREXIT(cinfo, JERR_INPUT_EMPTY);

		/*
		 * Insert fake EOI so that we proceed with whater has been read
		 * so far, likely the libjpeg will emit premature end of data
		 * segment or other kind of error but we may as well happen to
		 * load slightly corrupted image.
		 */
		char *b = src->buffer;
		b[0] = 0xff;
		b[1] = JPEG_EOI;
		ret = 2;
	}

	src->mgr.next_input_byte = src->buffer;
	src->mgr.bytes_in_buffer = ret;
	src->start_of_file = 0;

	return TRUE;
}

static void skip_input_data(struct jpeg_decompress_struct *cinfo, long num_bytes)
{
	struct my_source_mgr* src = (void*)cinfo->src;
	off_t ret;

	GP_DEBUG(3, "Skipping %li bytes", num_bytes);

	if (src->mgr.bytes_in_buffer < (unsigned long)num_bytes) {
		ret = gp_io_seek(src->io, num_bytes - src->mgr.bytes_in_buffer, GP_SEEK_CUR);
		//TODO: Call jpeg error
		if (ret == (off_t)-1)
			GP_FATAL("Failed to skip data: %s", strerror(errno));
		src->mgr.bytes_in_buffer = 0;
	} else {
		src->mgr.bytes_in_buffer -= num_bytes;
		src->mgr.next_input_byte += num_bytes;
	}
}

static inline void init_source_mgr(struct my_source_mgr *src, gp_io *io,
                                   void *buf, size_t buf_size)
{
	src->mgr.init_source = dummy_src;
	src->mgr.resync_to_restart = jpeg_resync_to_restart;
	src->mgr.term_source = dummy_src;
	src->mgr.fill_input_buffer = fill_input_buffer;
	src->mgr.skip_input_data = skip_input_data;
	src->mgr.bytes_in_buffer = 0;
	src->mgr.next_input_byte = NULL;
	src->io = io;
	src->buffer = buf;
	src->size = buf_size;
	src->start_of_file = 1;
}

#define JPEG_COM_MAX 128

static void read_jpg_metadata(struct jpeg_decompress_struct *cinfo,
                              gp_storage *storage, gp_correction_desc *corr_desc)
{
	jpeg_saved_marker_ptr marker;

	gp_storage_add_int(storage, NULL, "Width", cinfo->image_width);
	gp_storage_add_int(storage, NULL, "Height", cinfo->image_height);
	gp_storage_add_int(storage, NULL, "Channels", cinfo->num_components);
	gp_storage_add_string(storage, NULL, "Color Space",
	                     get_colorspace(cinfo->out_color_space));

	for (marker = cinfo->marker_list; marker != NULL; marker = marker->next) {
		switch (marker->marker) {
		case JPEG_COM: {
			//TODO: Is comment NULL terminated?
			char buf[marker->data_length+1];

			GP_DEBUG(3, "JPEG_COM comment block, size %u",
			         (unsigned int)marker->data_length);

			memcpy(buf, marker->data, marker->data_length);
			buf[marker->data_length] = '\0';
			gp_storage_add_string(storage, NULL, "Comment", buf);
		} break;
		case JPEG_APP0:
			GP_TODO("JFIF");
		break;
		case JPEG_APP0 + 1: {
			/*
			 * Make sure we parse only Exif and not Adobe XMP that
			 * reuses the same marker.
			 */
			if (marker->data[0] != 'E' || marker->data[1] != 'x' ||
			    marker->data[2] != 'i' || marker->data[3] != 'f')
				continue;

			gp_io *io = gp_io_mem(marker->data, marker->data_length, NULL);
			if (!io) {
				GP_WARN("Failed to create MemIO");
			} else {
				gp_read_exif(io, storage, corr_desc);
				gp_io_close(io);
			}
		}
		break;
		case JPEG_APP0 + 2: {
			if (marker->data_length <= 14) {
				GP_DEBUG(1, "APP2 block unexpectedly short.");
				continue;
			}

			if (memcmp(marker->data, "ICC_PROFILE", 12)) {
				GP_DEBUG(1, "APP2 block does not start with ICC_PROFILE");
				continue;
			}

			printf("%x %x\n", marker->data[12], marker->data[13]);

			gp_io *io = gp_io_mem(marker->data + 14, marker->data_length - 14, NULL);
			if (!io) {
				GP_WARN("Failed to create MemIO");
			} else {
				gp_read_icc(io, storage);
				gp_io_close(io);
			}
		} break;
		}
	}
}

static void save_jpg_markers(struct jpeg_decompress_struct *cinfo)
{
	/* Comment marker */
	jpeg_save_markers(cinfo, JPEG_COM, JPEG_COM_MAX);

	/* APP0 marker = JFIF data */
	jpeg_save_markers(cinfo, JPEG_APP0, 0xffff);

	/* APP1 marker = Exif data */
	jpeg_save_markers(cinfo, JPEG_APP0 + 1, 0xffff);

	/* APP2 marker = ICC profile */
	jpeg_save_markers(cinfo, JPEG_APP0 + 2, 0xffff);
}

int gp_read_jpg_ex(gp_io *io, gp_pixmap **img,
		 gp_storage *storage, gp_progress_cb *callback)
{
	struct jpeg_decompress_struct cinfo;
	struct my_source_mgr src;
	struct my_jpg_err my_err;
	gp_pixmap *ret = NULL;
	gp_correction_desc corr_desc = {.corr_type = -1};
	uint8_t buf[1024];
	int err;

	cinfo.err = jpeg_std_error(&my_err.error_mgr);
	my_err.error_mgr.error_exit = my_error_exit;

	if (setjmp(my_err.setjmp_buf)) {
		err = EIO;
		goto err2;
	}

	jpeg_create_decompress(&cinfo);
	init_source_mgr(&src, io, buf, sizeof(buf));
	cinfo.src = (void*)&src;

	save_jpg_markers(&cinfo);

	jpeg_read_header(&cinfo, TRUE);

	GP_DEBUG(1, "Have %s JPEG size %ux%u %i channels",
	            get_colorspace(cinfo.jpeg_color_space),
	            cinfo.image_width, cinfo.image_height,
		    cinfo.num_components);

	read_jpg_metadata(&cinfo, storage, &corr_desc);

	if (!img)
		goto exit;

	gp_pixel pixel_type;

	switch (cinfo.out_color_space) {
	case JCS_GRAYSCALE:
		pixel_type = GP_PIXEL_G8;
	break;
	case JCS_RGB:
		pixel_type = GP_PIXEL_BGR888;
	break;
	case JCS_CMYK:
		pixel_type = GP_PIXEL_CMYK8888;
	break;
	default:
		pixel_type = GP_PIXEL_UNKNOWN;
	}

	if (pixel_type == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Can't handle %s JPEG output format",
		            get_colorspace(cinfo.out_color_space));
		err = ENOSYS;
		goto err1;
	}

	ret = gp_pixmap_alloc(cinfo.image_width, cinfo.image_height,
			      pixel_type);

	if (!ret) {
		GP_DEBUG(1, "Malloc failed :(");
		err = ENOMEM;
		goto err1;
	}

	switch (pixel_type) {
	case GP_PIXEL_BGR888:
	case GP_PIXEL_G8:
		if (corr_desc.corr_type == -1)
			corr_desc.corr_type = GP_CORRECTION_TYPE_SRGB;

		gp_pixmap_correction_set(ret, &corr_desc);
	break;
	default:
	break;
	}

	jpeg_start_decompress(&cinfo);

	switch (pixel_type) {
	case GP_PIXEL_BGR888:
	case GP_PIXEL_G8:
		err = load(&cinfo, ret, callback);
	break;
	case GP_PIXEL_CMYK8888:
		err = load_cmyk(&cinfo, ret, callback);
	break;
	default:
		err = EINVAL;
	}

	if (err)
		goto err2;

	jpeg_finish_decompress(&cinfo);
exit:
	jpeg_destroy_decompress(&cinfo);

	gp_progress_cb_done(callback);

	if (img)
		*img = ret;

	return 0;
err2:
	gp_pixmap_free(ret);
err1:
	jpeg_destroy_decompress(&cinfo);
	errno = err;
	return 1;
}

static int save_convert(struct jpeg_compress_struct *cinfo,
                        const gp_pixmap *src,
                        gp_pixel_type out_pix,
                        gp_progress_cb *callback)
{
	uint8_t tmp[(src->w * gp_pixel_size(out_pix)) / 8 + 1];
	gp_line_convert convert;

	convert = gp_line_convert_get(src->pixel_type, out_pix);

	while (cinfo->next_scanline < cinfo->image_height) {
		uint32_t y = cinfo->next_scanline;
		void *in = GP_PIXEL_ADDR(src, 0, y);

		convert(in, tmp, src->w);

		JSAMPROW row = (void*)tmp;
		jpeg_write_scanlines(cinfo, &row, 1);

		if (gp_progress_cb_report(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static int save(struct jpeg_compress_struct *cinfo,
                const gp_pixmap *src,
                gp_progress_cb *callback)
{
	while (cinfo->next_scanline < cinfo->image_height) {
		uint32_t y = cinfo->next_scanline;

		JSAMPROW row = (void*)GP_PIXEL_ADDR(src, 0, y);
		jpeg_write_scanlines(cinfo, &row, 1);

		if (gp_progress_cb_report(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

struct my_dest_mgr {
	struct jpeg_destination_mgr mgr;
	void *buffer;
	ssize_t size;
	gp_io *io;
};

static void dummy_dst(j_compress_ptr GP_UNUSED(cinfo))
{
}

static boolean empty_output_buffer(j_compress_ptr cinfo)
{
	struct my_dest_mgr *dest = (void*)cinfo->dest;

	if (gp_io_write(dest->io, dest->buffer, dest->size) != dest->size) {
		GP_DEBUG(1, "Failed to write JPEG buffer");
		return FALSE;
	}

	dest->mgr.next_output_byte = dest->buffer;
	dest->mgr.free_in_buffer = dest->size;

	return TRUE;
}

static void term_destination(j_compress_ptr cinfo)
{
	struct my_dest_mgr *dest = (void*)cinfo->dest;
	ssize_t to_write = dest->size - dest->mgr.free_in_buffer;

	if (to_write > 0) {
		if (gp_io_write(dest->io, dest->buffer, to_write) != to_write) {
			GP_DEBUG(1, "Failed to write JPEG buffer");
			//TODO: Error handling
			return;
		}
	}
}

static inline void init_dest_mgr(struct my_dest_mgr *dst, gp_io *io,
                                 void *buf, size_t buf_size)
{
	dst->mgr.init_destination = dummy_dst;
	dst->mgr.empty_output_buffer = empty_output_buffer;
	dst->mgr.term_destination = term_destination;
	dst->mgr.next_output_byte = buf;
	dst->mgr.free_in_buffer = buf_size;

	dst->io = io;
	dst->buffer = buf;
	dst->size = buf_size;
}

static gp_pixel_type out_pixel_types[] = {
	GP_PIXEL_BGR888,
	GP_PIXEL_G8,
	GP_PIXEL_UNKNOWN
};

int gp_write_jpg(const gp_pixmap *src, gp_io *io,
                gp_progress_cb *callback)
{
	struct jpeg_compress_struct cinfo;
	gp_pixel_type out_pix;
	struct my_jpg_err my_err;
	struct my_dest_mgr dst;
	uint8_t buf[1024];
	int err;

	GP_DEBUG(1, "Writing JPG Image to I/O (%p)", io);

	out_pix = gp_line_convertible(src->pixel_type, out_pixel_types);

	if (out_pix == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unsupported pixel type %s",
		         gp_pixel_type_name(src->pixel_type));
		errno = ENOSYS;
		return 1;
	}

	if (setjmp(my_err.setjmp_buf)) {
		errno = EIO;
		return 1;

	}

	cinfo.err = jpeg_std_error(&my_err.error_mgr);
	my_err.error_mgr.error_exit = my_error_exit;

	jpeg_create_compress(&cinfo);

	init_dest_mgr(&dst, io, buf, sizeof(buf));
	cinfo.dest = (void*)&dst;

	cinfo.image_width  = src->w;
	cinfo.image_height = src->h;

	switch (out_pix) {
	case GP_PIXEL_BGR888:
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
	break;
	case GP_PIXEL_G8:
		cinfo.input_components = 1;
		cinfo.in_color_space = JCS_GRAYSCALE;
	break;
	default:
		GP_BUG("Don't know how to set color_space and compoments");
	}

	jpeg_set_defaults(&cinfo);

	jpeg_start_compress(&cinfo, TRUE);

	if (out_pix != src->pixel_type)
		err = save_convert(&cinfo, src, out_pix, callback);
	else
		err = save(&cinfo, src, callback);

	if (err) {
		jpeg_destroy_compress(&cinfo);
		errno = err;
		return 1;
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	gp_progress_cb_done(callback);
	return 0;
}

#else

int gp_read_jpg_ex(gp_io GP_UNUSED(*io), gp_pixmap GP_UNUSED(**img),
                 gp_storage GP_UNUSED(*storage),
                 gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

int gp_write_jpg(const gp_pixmap GP_UNUSED(*src), gp_io GP_UNUSED(*io),
                gp_progress_cb GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_JPEG */

const gp_loader gp_jpg = {
#ifdef HAVE_JPEG
	.read = gp_read_jpg_ex,
	.write = gp_write_jpg,
	.save_ptypes = out_pixel_types,
#endif
	.match = gp_match_jpg,

	.fmt_name = "JPEG",
	.extensions = {"jpg", "jpeg", NULL},
};
