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

  General functions for loading and saving bitmaps.
  
 */

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "core/GP_Debug.h"

#include "GP_Loaders.h"

#include "GP_Loader.h"

static GP_Loader psp_loader = {
	.Load = GP_LoadPSP,
	.Save = NULL,
	.Match = GP_MatchPSP,
	.fmt_name = "Paint Shop Pro Image",
	.next = NULL,
	.extensions = {"psp", "pspimage", NULL},
};

static GP_Loader bmp_loader = {
	.Load = GP_LoadBMP,
	.Save = NULL,
	.Match = NULL,
	.fmt_name = "BMP",
	.next = &psp_loader,
	.extensions = {"bmp", "dib", NULL},
};

static GP_Loader gif_loader = {
	.Load = GP_LoadGIF,
	.Save = NULL,
	.Match = GP_MatchGIF,
	.fmt_name = "Graphics Interchange Format",
	.next = &bmp_loader,
	.extensions = {"gif", NULL},
};

static GP_Loader png_loader = {
	.Load = GP_LoadPNG,
	.Save = GP_SavePNG,
	.Match = GP_MatchPNG,
	.fmt_name = "Portable Network Graphics",
	.next = &gif_loader,
	.extensions = {"png", NULL},
};

static GP_Loader jpeg_loader = {
	.Load = GP_LoadJPG,
	.Save = GP_SaveJPG,
	.Match = GP_MatchJPG,
	.fmt_name = "JPEG",
	.next = &png_loader,
	.extensions = {"jpg", "jpeg", NULL},
};

static GP_Loader *loaders = &jpeg_loader;
static GP_Loader *loaders_last = &psp_loader;

void GP_LoaderRegister(GP_Loader *self)
{
	GP_DEBUG(1, "Registering loader for '%s'", self->fmt_name);

	self->next = NULL;
	loaders_last->next = self;
}

void GP_LoaderUnregister(GP_Loader *self)
{
	struct GP_Loader *i;

	if (self == NULL)
		return;

	GP_DEBUG(1, "Unregistering loader for '%s'", self->fmt_name);

	for (i = loaders; i != NULL; i = i->next) {
		if (i->next == self)
			break;
	}

	if (i == NULL) {
		GP_WARN("Loader '%s' (%p) wasn't registered",
		        self->fmt_name, self);
		return;
	}

	i->next = self->next;
}

static struct GP_Loader *loader_by_extension(const char *ext)
{
	struct GP_Loader *i;
	int j;

	for (i = loaders; i != NULL; i = i->next) {
		for (j = 0; i->extensions[j] != NULL; j++) {
			if (!strcasecmp(ext, i->extensions[j])) {
				GP_DEBUG(1, "Found loader '%s'", i->fmt_name);
				return i;
			}
		}
	}

	return NULL;
}

static struct GP_Loader *loader_by_filename(const char *path)
{
	size_t len = strlen(path);
	const char *ext;
	int i;

	for (i = len - 1; i >= 0; i--)
		if (path[i] == '.')
			break;

	ext = path + i + 1;

	GP_DEBUG(1, "Loading file by filename extension '%s'", ext);

	return loader_by_extension(ext);
}

static struct GP_Loader *loader_by_signature(const char *path)
{
	uint8_t buf[32];
	FILE *f;
	int err;

	GP_DEBUG(1, "Trying to match file signature");

	f = fopen(path, "rb");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open file '%s'", path);
		errno = err;
		return NULL;
	}

	if (fread(buf, sizeof(buf), 1, f) < 1) {
		GP_DEBUG(1, "Failed to read start of the file '%s'", path);
		errno = EIO;
		return NULL;
	}

	fclose(f);

	struct GP_Loader *i;
	
	for (i = loaders; i != NULL; i = i->next) {
		if (i->Match && i->Match(buf)) {
			GP_DEBUG(1, "Found loader '%s'", i->fmt_name);
			return i;
		}
	}

	return NULL;
}

enum GP_ImageFmt {
	GP_FMT_UNKNOWN,
	GP_FMT_PNG,
	GP_FMT_JPG,
	GP_FMT_BMP,
	GP_FMT_GIF,
	GP_FMT_PSP,
	GP_FMT_PBM,
	GP_FMT_PGM,
	GP_FMT_PPM,
};

enum GP_ImageFmt filename_to_fmt(const char *path)
{
	size_t len = strlen(path);

	if (len < 3)
		return GP_FMT_UNKNOWN;

	switch (path[len - 1]) {
	/* PNG, JPG, JPEG */
	case 'g':
	case 'G':
		switch (path[len - 2]) {
		case 'n':
		case 'N':
			if (path[len - 3] == 'p' ||
			    path[len - 3] == 'P')
				return GP_FMT_PNG;
		break;
		case 'p':
		case 'P':
			if (path[len - 3] == 'j' ||
			    path[len - 3] == 'J')
				return GP_FMT_JPG;
		break;
		case 'e':
		case 'E':
			if ((path[len - 3] == 'p' ||
			     path[len - 3] == 'P') &&
			    (path[len - 4] == 'j' ||
			     path[len - 4] == 'J'))
				return GP_FMT_JPG;
		break;
		}
	break;
	/* PPM, PGM, PBM, PNM */
	case 'm':
	case 'M':
		switch (path[len - 2]) {
		case 'b':
		case 'B':
			if (path[len - 3] == 'p' ||
			    path[len - 3] == 'P')
				return GP_FMT_PBM;
		break;
		case 'g':
		case 'G':
			if (path[len - 3] == 'p' ||
			    path[len - 3] == 'P')
				return GP_FMT_PGM;
		break;
		case 'p':
		case 'P':
			if (path[len - 3] == 'p' ||
			    path[len - 3] == 'P')
				return GP_FMT_PPM;
		break;
		}
	break;
	/* BMP, PSP */
	case 'P':
	case 'p':
		switch (path[len - 2]) {
		case 'M':
		case 'm':
			if (path[len - 3] == 'B' ||
			    path[len - 3] == 'b')
				return GP_FMT_BMP;
		break;
		case 'S':
		case 's':
			if (path[len - 3] == 'P' ||
			    path[len - 3] == 'p')
				return GP_FMT_PSP;
		break;
		}
	break;
	/* GIF */
	case 'F':
	case 'f':
		switch (path[len - 2]) {
		case 'I':
		case 'i':
			if (path[len - 3] == 'G' ||
			    path[len - 3] == 'g')
				return GP_FMT_GIF;
		break;
		}
	break;
	}

	return GP_FMT_UNKNOWN;
}

GP_Context *GP_LoadImage(const char *src_path, GP_ProgressCallback *callback)
{
	int saved_errno;
	
	if (access(src_path, R_OK)) {
		
		saved_errno = errno;

		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		
		errno = saved_errno;
		
		return NULL;
	}
	
	enum GP_ImageFmt fmt = filename_to_fmt(src_path);

	switch (fmt) {
	case GP_FMT_JPG:
		return GP_LoadJPG(src_path, callback);
	case GP_FMT_PNG:
		return GP_LoadPNG(src_path, callback);
	case GP_FMT_GIF:
		return GP_LoadGIF(src_path, callback);
	case GP_FMT_PSP:
		return GP_LoadPSP(src_path, callback);
	case GP_FMT_BMP:
		return GP_LoadBMP(src_path, callback);
	case GP_FMT_PBM:
		return GP_LoadPBM(src_path, callback);
	case GP_FMT_PGM:
		return GP_LoadPGM(src_path, callback);
	case GP_FMT_PPM:
		return GP_LoadPPM(src_path, callback);
	case GP_FMT_UNKNOWN:
	break;
	}

	struct GP_Loader *l;
	
	l = loader_by_filename(src_path);

	if (l != NULL)
		return l->Load(src_path, callback);

	l = loader_by_signature(src_path);

	if (l != NULL)
		return l->Load(src_path, callback);

	//TODO file signature based check
	errno = ENOSYS;
	return NULL;
}

int GP_LoadMetaData(const char *src_path, GP_MetaData *data)
{
	int saved_errno;
	
	if (access(src_path, R_OK)) {
		
		saved_errno = errno;

		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		
		errno = saved_errno;
		
		return 1;
	}

	enum GP_ImageFmt fmt = filename_to_fmt(src_path);

	switch (fmt) {
	case GP_FMT_JPG:
		return GP_LoadJPGMetaData(src_path, data);
	case GP_FMT_PNG:
		return GP_LoadPNGMetaData(src_path, data);
	
	default:
	break;
	}

	errno = ENOSYS;
	return 1;
}

int GP_SaveImage(const GP_Context *src, const char *dst_path,
                 GP_ProgressCallback *callback)
{
	enum GP_ImageFmt fmt = filename_to_fmt(dst_path);

	switch (fmt) {
	case GP_FMT_JPG:
		return GP_SaveJPG(src, dst_path, callback);
	case GP_FMT_PNG:
		return GP_SavePNG(src, dst_path, callback);

	default:
	break;
	}

	errno = ENOSYS;
	return 1;
}
