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
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

/*

  General functions for loading and saving bitmaps.
  
 */

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

static GP_Loader pbm_loader = {
	.Load = GP_LoadPBM,
	.Save = GP_SavePBM,
	.Match = NULL,
	.fmt_name = "Netpbm portable bitmap",
	.next = &psp_loader,
	.extensions = {"pbm", NULL},
};

static GP_Loader pgm_loader = {
	.Load = GP_LoadPGM,
	.Save = GP_SavePGM,
	.Match = NULL,
	.fmt_name = "Netpbm portable graymap",
	.next = &pbm_loader,
	.extensions = {"pgm", NULL},
};

static GP_Loader ppm_loader = {
	.Load = GP_LoadPPM,
	.Save = NULL,
	.Match = NULL,
	.fmt_name = "Netpbm portable pixmap",
	.next = &pgm_loader,
	.extensions = {"ppm", NULL},
};

static GP_Loader bmp_loader = {
	.Load = GP_LoadBMP,
	.Save = GP_SaveBMP,
	.Match = GP_MatchBMP,
	.fmt_name = "BMP",
	.next = &ppm_loader,
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

static GP_Loader tiff_loader = {
	.Load = GP_LoadTIFF,
	.Save = NULL,
	.Match = GP_MatchTIFF,
	.fmt_name = "Tag Image File Format",
	.next = &gif_loader,
	.extensions = {"tif", "tiff", NULL},
};

static GP_Loader png_loader = {
	.Load = GP_LoadPNG,
	.Save = GP_SavePNG,
	.Match = GP_MatchPNG,
	.fmt_name = "Portable Network Graphics",
	.next = &tiff_loader,
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

void GP_ListLoaders(void)
{
	struct GP_Loader *i;
	int j;

	for (i = loaders; i != NULL; i = i->next) {
		printf("Format: %s\n", i->fmt_name);
		printf("Load:\t%s\n", i->Load ? "Yes" : "No");
		printf("Save:\t%s\n", i->Save ? "Yes" : "No");
		printf("Match:\t%s\n", i->Match ? "Yes" : "No");
		printf("Extensions: ");
		for (j = 0; i->extensions[j] != NULL; j++)
			printf("%s ", i->extensions[j]);
		printf("\n");
	
		if (i->next != NULL)
			printf("\n");
	}
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

	if (path[i] != '.')
		return NULL;

	ext = path + i + 1;

	GP_DEBUG(1, "Loading file by filename extension '%s'", ext);

	return loader_by_extension(ext);
}

static const GP_Loader *loader_by_signature(const char *path)
{
	uint8_t buf[32];
	FILE *f;
	int err;

	GP_DEBUG(1, "Trying to load a file by signature");

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

	return GP_MatchSignature(buf);
}

GP_Context *GP_LoadImage(const char *src_path, GP_ProgressCallback *callback)
{
	int saved_errno;
	struct stat st;
	
	if (access(src_path, R_OK)) {
		
		saved_errno = errno;

		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		
		errno = saved_errno;
		
		return NULL;
	}

	if (stat(src_path, &st)) {
		GP_WARN("Failed to stat '%s': %s", src_path, strerror(errno));
	} else {
		if (st.st_mode & S_IFDIR) {
			errno = EISDIR;
			return NULL;
		}
	}

	GP_Context *img;
	const GP_Loader *ext_load = NULL, *sig_load;

	ext_load = loader_by_filename(src_path);

	if (ext_load != NULL && ext_load->Load != NULL) {
		img = ext_load->Load(src_path, callback);
		
		if (img)
			return img;
	}

	sig_load = loader_by_signature(src_path);

	/* 
	 * Avoid further work if extension matches the signature but image
	 * couldn't be loaded. Probably unimplemented format or damaged file.
	 */
	if (ext_load == sig_load)
		return NULL;

	if (ext_load && sig_load) {
		GP_WARN("File '%s': Extension says %s but signature %s",
			src_path, ext_load->fmt_name, sig_load->fmt_name);
	}

	if (sig_load && sig_load->Load != NULL)
		return sig_load->Load(src_path, callback);

	errno = ENOSYS;
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

static enum GP_ImageFmt filename_to_fmt(const char *path)
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
	struct GP_Loader *l = loader_by_filename(dst_path);

	if (l == NULL) {
		errno = EINVAL;
		return 1;
	}
	
	if (l->Save)
		return l->Save(src, dst_path, callback);

	errno = ENOSYS;
	return 1;
}

const GP_Loader *GP_MatchSignature(const void *buf)
{
	struct GP_Loader *i;
	
	for (i = loaders; i != NULL; i = i->next) {
		if (i->Match && i->Match(buf) == 1) {
			GP_DEBUG(1, "Found loader '%s'", i->fmt_name);
			return i;
		}
	}

	GP_DEBUG(1, "Loader not found");

	return NULL;
}

const GP_Loader *GP_MatchExtension(const char *path)
{
	return loader_by_filename(path);
}
