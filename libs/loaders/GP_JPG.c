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
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
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

#include "GP_JPG.h"

#ifdef HAVE_JPEG

#include <jpeglib.h>

/*
 * 0xff 0xd8 - start of image
 * 0xff 0xe0 - APP0 JFIF meta data
 */
#define JPEG_SIGNATURE "\xff\xd8\xff\xe0"
#define JPEG_SIGNATURE_LEN 4

int GP_MatchJPG(const void *buf)
{
	return !memcmp(buf, JPEG_SIGNATURE, JPEG_SIGNATURE_LEN);
}

int GP_OpenJPG(const char *src_path, FILE **f)
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

GP_Context *GP_ReadJPG(FILE *f, GP_ProgressCallback *callback)
{
	struct jpeg_decompress_struct cinfo;
	struct my_jpg_err my_err;
	GP_Context *ret = NULL;
	int err;

	cinfo.err = jpeg_std_error(&my_err.error_mgr);
	my_err.error_mgr.error_exit = my_error_exit;

	if (setjmp(my_err.setjmp_buf)) {
		err = EIO;
		goto err2;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, f);

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
		pixel_type = GP_PIXEL_RGB888;
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

	while (cinfo.output_scanline < cinfo.output_height) {
		uint32_t y = cinfo.output_scanline;

		JSAMPROW addr = (void*)GP_PIXEL_ADDR(ret, 0, y);
		jpeg_read_scanlines(&cinfo, &addr, 1);

		if (pixel_type != GP_PIXEL_RGB888)
			continue;
		
		//TODO: fixme bigendian?
		/* fix the pixel, as we want in fact BGR */
		uint32_t i;
		
		for (i = 0; i < ret->w; i++) {
			uint8_t *pix = GP_PIXEL_ADDR(ret, i, y);
			GP_SWAP(pix[0], pix[2]);
		}
	
		if (GP_ProgressCallbackReport(callback, y, ret->h, ret->w)) {
			GP_DEBUG(1, "Operation aborted");
			err = ECANCELED;
			goto err2;
		}
	}

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
	FILE *f;
	GP_Context *res;

	if (GP_OpenJPG(src_path, &f))
		return NULL;

	res = GP_ReadJPG(f, callback);
	
	fclose(f);

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
	FILE *f;
	int ret;

	if (GP_OpenJPG(src_path, &f))
		return 1;

	ret = GP_ReadJPGMetaData(f, data);

	fclose(f);

	return ret;
}

int GP_SaveJPG(const GP_Context *src, const char *dst_path,
               GP_ProgressCallback *callback)
{
	FILE *f;
	struct jpeg_compress_struct cinfo;
	struct my_jpg_err my_err;
	int err;

	GP_DEBUG(1, "Saving JPG Image '%s'", dst_path);

	if (src->pixel_type != GP_PIXEL_RGB888 &&
	    src->pixel_type != GP_PIXEL_G8) {
		GP_DEBUG(1, "Can't save png with pixel type %s",
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
	cinfo.input_components = src->pixel_type == GP_PIXEL_RGB888 ? 3 : 1;
	cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);

	jpeg_start_compress(&cinfo, TRUE);

	while (cinfo.next_scanline < cinfo.image_height) {
		uint32_t y = cinfo.next_scanline;

		if (src->pixel_type == GP_PIXEL_RGB888) {
			uint32_t i;
			uint8_t tmp[3 * src->w];

			memcpy(tmp, GP_PIXEL_ADDR(src, 0, y), 3 * src->w);

			/* fix the pixels as we want in fact BGR */
			for (i = 0; i < src->w; i++) {
				uint8_t *pix = tmp + 3 * i; 
				GP_SWAP(pix[0], pix[2]);
			}

			JSAMPROW row = (void*)tmp;
			jpeg_write_scanlines(&cinfo, &row, 1);
		} else {
			JSAMPROW row = (void*)GP_PIXEL_ADDR(src, 0, y);
			jpeg_write_scanlines(&cinfo, &row, 1);
		}
	
		if (GP_ProgressCallbackReport(callback, y, src->h, src->w)) {
			GP_DEBUG(1, "Operation aborted");
			err = ECANCELED;
			goto err3;
		}
	}

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

int GP_OpenJPG(const char GP_UNUSED(*src_path), FILE GP_UNUSED(**f))
{
	errno = ENOSYS;
	return 1;
}

GP_Context *GP_ReadJPG(FILE GP_UNUSED(*f),
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
