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

  JPG image support using jpeg library.

 */

#include <stdint.h>
#include <inttypes.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

#include "../../config.h"
#include "core/GP_Debug.h"

#include "loaders/GP_LineConvert.h"
#include "loaders/GP_JPG.h"

#ifdef HAVE_JPEG

#include <jpeglib.h>

/*
 * 0xff 0xd8 - start of image
 * 0xff 0x.. - start of frame
 */
#define JPEG_SIGNATURE "\xff\xd8\xff"
#define JPEG_SIGNATURE_LEN 3

int GP_MatchJPG(const void *buf)
{
	return !memcmp(buf, JPEG_SIGNATURE, JPEG_SIGNATURE_LEN);
}

struct my_jpg_err {
	struct jpeg_error_mgr error_mgr;
	jmp_buf setjmp_buf;
};

static void my_error_exit(j_common_ptr cinfo)
{
	struct my_jpg_err *my_err = (struct my_jpg_err*) cinfo->err;

	GP_DEBUG(1, "ERROR reading/writing jpeg file");

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

static int load(struct jpeg_decompress_struct *cinfo, GP_Context *ret,
                GP_ProgressCallback *callback)
{
	while (cinfo->output_scanline < cinfo->output_height) {
		uint32_t y = cinfo->output_scanline;
                JSAMPROW addr = (void*)GP_PIXEL_ADDR(ret, 0, y);

		jpeg_read_scanlines(cinfo, &addr, 1);

		if (GP_ProgressCallbackReport(callback, y, ret->h, ret->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static int load_cmyk(struct jpeg_decompress_struct *cinfo, GP_Context *ret,
                       GP_ProgressCallback *callback)
{
	while (cinfo->output_scanline < cinfo->output_height) {
		uint32_t y = cinfo->output_scanline;

		JSAMPROW addr = (void*)GP_PIXEL_ADDR(ret, 0, y);
		jpeg_read_scanlines(cinfo, &addr, 4);

		unsigned int i;
		uint8_t *buf = GP_PIXEL_ADDR(ret, 0, y);

		for (i = 0; i < ret->w; i++) {
			unsigned int j = 4 * i;

			buf[j]   = 0xff - buf[j];
			buf[j+1] = 0xff - buf[j+1];
			buf[j+2] = 0xff - buf[j+2];
			buf[j+3] = 0xff - buf[j+3];
		}

		if (GP_ProgressCallbackReport(callback, y, ret->h, ret->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}


struct my_source_mgr {
	struct jpeg_source_mgr mgr;
	void *buffer;
	size_t size;
	GP_IO *io;
};

static void dummy(j_decompress_ptr GP_UNUSED(cinfo))
{
}

static boolean fill_input_buffer(struct jpeg_decompress_struct *cinfo)
{
	int ret;
	struct my_source_mgr* src = (void*)cinfo->src;

	ret = GP_IORead(src->io, src->buffer, src->size);

	if (ret < 0) {
		GP_WARN("Failed to fill buffer");
		return 0;
	}

	src->mgr.next_input_byte = src->buffer;
	src->mgr.bytes_in_buffer = ret;
	return 1;
}

static void skip_input_data(struct jpeg_decompress_struct *cinfo, long num_bytes)
{
	struct my_source_mgr* src = (void*)cinfo->src;
	off_t ret;

	GP_DEBUG(3, "Skipping %li bytes", num_bytes);

	if (src->mgr.bytes_in_buffer < (unsigned long)num_bytes) {
		ret = GP_IOSeek(src->io, num_bytes - src->mgr.bytes_in_buffer, GP_IO_SEEK_CUR);
		//TODO: Call jpeg error
		if (ret == (off_t)-1)
			GP_FATAL("Failed to skip data: %s", strerror(errno));
		src->mgr.bytes_in_buffer = 0;
	} else {
		src->mgr.bytes_in_buffer -= num_bytes;
		src->mgr.next_input_byte += num_bytes;
	}
}

GP_Context *GP_ReadJPG(GP_IO *io, GP_ProgressCallback *callback)
{
	struct jpeg_decompress_struct cinfo;
	struct my_source_mgr src;
	struct my_jpg_err my_err;
	GP_Context *ret = NULL;
	uint8_t buffer[1024];
	int err;

	cinfo.err = jpeg_std_error(&my_err.error_mgr);
	my_err.error_mgr.error_exit = my_error_exit;

	if (setjmp(my_err.setjmp_buf)) {
		err = EIO;
		goto err2;
	}

	jpeg_create_decompress(&cinfo);

	/* Initialize custom source manager */
	src.mgr.init_source = dummy;
	src.mgr.resync_to_restart = jpeg_resync_to_restart;
	src.mgr.term_source = dummy;
	src.mgr.fill_input_buffer = fill_input_buffer;
	src.mgr.skip_input_data = skip_input_data;
	src.mgr.bytes_in_buffer = 0;
	src.mgr.next_input_byte = NULL;
	src.io = io;
	src.buffer = buffer;
	src.size = sizeof(buffer);
	cinfo.src = (void*)&src;

	jpeg_read_header(&cinfo, TRUE);

	GP_DEBUG(1, "Have %s JPEG size %ux%u %i channels",
	            get_colorspace(cinfo.jpeg_color_space),
	            cinfo.image_width, cinfo.image_height,
		    cinfo.num_components);

	GP_Pixel pixel_type;

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

	ret = GP_ContextAlloc(cinfo.image_width, cinfo.image_height,
	                      pixel_type);

	if (ret == NULL) {
		GP_DEBUG(1, "Malloc failed :(");
		err = ENOMEM;
		goto err1;
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
	jpeg_destroy_decompress(&cinfo);

	GP_ProgressCallbackDone(callback);

	return ret;
err2:
	GP_ContextFree(ret);
err1:
	jpeg_destroy_decompress(&cinfo);
	errno = err;
	return NULL;
}

GP_Context *GP_LoadJPG(const char *src_path, GP_ProgressCallback *callback)
{
	GP_IO *io;
	GP_Context *res;
	int err;

	io = GP_IOFile(src_path, GP_IO_RDONLY);
	if (!io)
		return NULL;

	res = GP_ReadJPG(io, callback);

	err = errno;
	GP_IOClose(io);
	errno = err;

	return res;
}

#define JPEG_COM_MAX 128

static void read_jpg_metadata(struct jpeg_decompress_struct *cinfo,
                              GP_MetaData *data)
{
	jpeg_saved_marker_ptr marker;

	for (marker = cinfo->marker_list; marker != NULL; marker = marker->next) {
		switch (marker->marker) {
		case JPEG_COM:
			GP_MetaDataCreateString(data, "comment", (void*)marker->data,
			                        marker->data_length, 1);
		break;
		case JPEG_APP0:
			GP_TODO("JFIF");
		break;
		case JPEG_APP0 + 1:
			GP_MetaDataFromExif(data, marker->data, marker->data_length);
		break;
		}
	}
}

static void save_jpg_markers(struct jpeg_decompress_struct *cinfo)
{
	/* Comment marker */
	jpeg_save_markers(cinfo, JPEG_COM, JPEG_COM_MAX);

	/* APP0 marker = JFIF data */
	jpeg_save_markers(cinfo, JPEG_APP0 + 1, 0xffff);

	/* APP1 marker = Exif data */
	jpeg_save_markers(cinfo, JPEG_APP0 + 1, 0xffff);
}

int GP_ReadJPGMetaData(FILE *f, GP_MetaData *data)
{
	struct jpeg_decompress_struct cinfo;
	struct my_jpg_err my_err;
	int err;

	cinfo.err = jpeg_std_error(&my_err.error_mgr);
	my_err.error_mgr.error_exit = my_error_exit;

	if (setjmp(my_err.setjmp_buf)) {
		err = EIO;
		goto err1;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);

	save_jpg_markers(&cinfo);

	jpeg_read_header(&cinfo, TRUE);

	GP_DEBUG(1, "Have %s JPEG size %ux%u %i channels",
	            get_colorspace(cinfo.jpeg_color_space),
	            cinfo.image_width, cinfo.image_height,
		    cinfo.num_components);

	read_jpg_metadata(&cinfo, data);

//	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 0;
err1:
	jpeg_destroy_decompress(&cinfo);
	errno = err;
	return 1;
}

int GP_LoadJPGMetaData(const char *src_path, GP_MetaData *data)
{
	//FILE *f;
	int ret = -1;

	//if (GP_OpenJPG(src_path, &f))
	//	return 1;

	//ret = GP_ReadJPGMetaData(f, data);

	//fclose(f);

	return ret;
}

static int save_convert(struct jpeg_compress_struct *cinfo,
                        const GP_Context *src,
                        GP_PixelType out_pix,
                        GP_ProgressCallback *callback)
{
	uint8_t tmp[(src->w * GP_PixelSize(out_pix)) / 8 + 1];
	GP_LineConvert Convert;

	Convert = GP_LineConvertGet(src->pixel_type, out_pix);

	while (cinfo->next_scanline < cinfo->image_height) {
		uint32_t y = cinfo->next_scanline;
		void *in = GP_PIXEL_ADDR(src, 0, y);

		Convert(in, tmp, src->w);

		JSAMPROW row = (void*)tmp;
		jpeg_write_scanlines(cinfo, &row, 1);

		if (GP_ProgressCallbackReport(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static int save(struct jpeg_compress_struct *cinfo,
                const GP_Context *src,
                GP_ProgressCallback *callback)
{
	while (cinfo->next_scanline < cinfo->image_height) {
		uint32_t y = cinfo->next_scanline;

		JSAMPROW row = (void*)GP_PIXEL_ADDR(src, 0, y);
		jpeg_write_scanlines(cinfo, &row, 1);

		if (GP_ProgressCallbackReport(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			return ECANCELED;
		}
	}

	return 0;
}

static GP_PixelType out_pixel_types[] = {
	GP_PIXEL_BGR888,
	GP_PIXEL_G8,
	GP_PIXEL_UNKNOWN
};

int GP_SaveJPG(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	FILE *f;
	struct jpeg_compress_struct cinfo;
	GP_PixelType out_pix;
	struct my_jpg_err my_err;
	int err;

	GP_DEBUG(1, "Saving JPG Image '%s'", dst_path);

	out_pix = GP_LineConvertible(src->pixel_type, out_pixel_types);

	if (out_pix == GP_PIXEL_UNKNOWN) {
		GP_DEBUG(1, "Unsupported pixel type %s",
		         GP_PixelTypeName(src->pixel_type));
		errno = ENOSYS;
		return 1;
	}

	f = fopen(dst_path, "wb");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open '%s' for writing: %s",
		         dst_path, strerror(errno));
		goto err0;
	}

	if (setjmp(my_err.setjmp_buf)) {
		err = EIO;
		//TODO: is cinfo allocated?
		goto err2;
	}

	cinfo.err = jpeg_std_error(&my_err.error_mgr);
	my_err.error_mgr.error_exit = my_error_exit;

	jpeg_create_compress(&cinfo);

	jpeg_stdio_dest(&cinfo, f);

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

	if (err)
		goto err3;

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	if (fclose(f)) {
		err = errno;
		GP_DEBUG(1, "Failed to close file '%s': %s",
		         dst_path, strerror(errno));
		goto err1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
err3:
	jpeg_destroy_compress(&cinfo);
err2:
	fclose(f);
err1:
	unlink(dst_path);
err0:
	errno = err;
	return 1;
}

#else

int GP_MatchJPG(const void GP_UNUSED(*buf))
{
	errno = ENOSYS;
	return -1;
}

int GP_OpenJPG(const char GP_UNUSED(*src_path), FILE GP_UNUSED(**f))
{
	errno = ENOSYS;
	return 1;
}

GP_Context *GP_ReadJPG(GP_IO GP_UNUSED(*io),
                       GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return NULL;
}

GP_Context *GP_LoadJPG(const char GP_UNUSED(*src_path),
                       GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return NULL;
}

int GP_ReadJPGMetaData(FILE GP_UNUSED(*f), GP_MetaData GP_UNUSED(*data))
{
	errno = ENOSYS;
	return 1;
}

int GP_LoadJPGMetaData(const char GP_UNUSED(*src_path),
                       GP_MetaData GP_UNUSED(*data))
{
	errno = ENOSYS;
	return 1;
}

int GP_SaveJPG(const GP_Context GP_UNUSED(*src),
               const char GP_UNUSED(*dst_path),
               GP_ProgressCallback GP_UNUSED(*callback))
{
	errno = ENOSYS;
	return 1;
}

#endif /* HAVE_JPEG */
