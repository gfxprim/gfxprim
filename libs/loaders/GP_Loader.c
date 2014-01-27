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

  General functions for loading and saving bitmaps.

 */

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include "core/GP_Debug.h"

#include "loaders/GP_Loaders.h"
#include "loaders/GP_Loader.h"

static GP_Loader psp_loader = {
	.Read = GP_ReadPSP,
	.Load = GP_LoadPSP,
	.Save = NULL,
	.Match = GP_MatchPSP,
	.fmt_name = "Paint Shop Pro Image",
	.next = NULL,
	.extensions = {"psp", "pspimage", NULL},
};

static GP_Loader pcx_loader = {
	.Read = GP_ReadPCX,
	.Load = GP_LoadPCX,
	.Save = NULL,
	.Match = GP_MatchPCX,
	.fmt_name = "ZSoft PCX",
	.next = &psp_loader,
	.extensions = {"pcx", NULL},
};

static GP_Loader pbm_loader = {
	.Read = GP_ReadPBM,
	.Load = GP_LoadPBM,
	.Save = GP_SavePBM,
	.Match = GP_MatchPBM,
	.fmt_name = "Netpbm portable Bitmap",
	.next = &pcx_loader,
	.extensions = {"pbm", NULL},
};

static GP_Loader pgm_loader = {
	.Read = GP_ReadPGM,
	.Load = GP_LoadPGM,
	.Save = GP_SavePGM,
	.Match = GP_MatchPGM,
	.fmt_name = "Netpbm portable Graymap",
	.next = &pbm_loader,
	.extensions = {"pgm", NULL},
};

static GP_Loader ppm_loader = {
	.Read = GP_ReadPPM,
	.Load = GP_LoadPPM,
	.Save = GP_SavePPM,
	.Match = GP_MatchPPM,
	.fmt_name = "Netpbm portable Pixmap",
	.next = &pgm_loader,
	.extensions = {"ppm", NULL},
};

static GP_Loader pnm_loader = {
	.Read = GP_ReadPNM,
	.Load = GP_LoadPNM,
	.Save = GP_SavePNM,
	/*
	 * Avoid double Match
	 * This format is covered by PBM, PGM and PPM
	 */
	.Match = NULL,
	.fmt_name = "Netpbm portable Anymap",
	.next = &ppm_loader,
	.extensions = {"pnm", NULL},
};

static GP_Loader jp2_loader = {
	.Read = GP_ReadJP2,
	.Load = GP_LoadJP2,
	.Save = NULL,
	.Match = GP_MatchJP2,
	.fmt_name = "JPEG 2000",
	.next = &pnm_loader,
	.extensions = {"jp2", "jpx", NULL},
};

static GP_Loader bmp_loader = {
	.Read = GP_ReadBMP,
	.Load = GP_LoadBMP,
	.Save = GP_SaveBMP,
	.Match = GP_MatchBMP,
	.fmt_name = "BMP",
	.next = &jp2_loader,
	.extensions = {"bmp", "dib", NULL},
};

static GP_Loader gif_loader = {
	.Read = GP_ReadGIF,
	.Load = GP_LoadGIF,
	.Save = NULL,
	.Match = GP_MatchGIF,
	.fmt_name = "Graphics Interchange Format",
	.next = &bmp_loader,
	.extensions = {"gif", NULL},
};

static GP_Loader tiff_loader = {
	.Read = GP_ReadTIFF,
	.Load = GP_LoadTIFF,
	.Save = GP_SaveTIFF,
	.Match = GP_MatchTIFF,
	.fmt_name = "Tag Image File Format",
	.next = &gif_loader,
	.extensions = {"tif", "tiff", NULL},
};

static GP_Loader png_loader = {
	.Read = GP_ReadPNG,
	.Load = GP_LoadPNG,
	.Save = GP_SavePNG,
	.Match = GP_MatchPNG,
	.fmt_name = "Portable Network Graphics",
	.next = &tiff_loader,
	.extensions = {"png", NULL},
};

static GP_Loader jpeg_loader = {
	.Read = GP_ReadJPG,
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
		printf("Read:\t%s\n", i->Read ? "Yes" : "No");
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

static const char *get_ext(const char *path)
{
	size_t len = strlen(path);
	int i;

	for (i = len - 1; i >= 0; i--)
		if (path[i] == '.')
			break;

	if (path[i] != '.')
		return NULL;

	return path + i + 1;
}

static struct GP_Loader *loader_by_filename(const char *path)
{
	const char *ext = get_ext(path);

	if (ext == NULL)
		return NULL;

	GP_DEBUG(1, "Loading file by filename extension '%s'", ext);

	return loader_by_extension(ext);
}

static const GP_Loader *loader_by_signature(const char *path)
{
	uint8_t buf[32];
	FILE *f;
	int err;

	const GP_Loader *ret;

	GP_DEBUG(1, "Trying to load by file signature");

	f = fopen(path, "rb");

	if (f == NULL) {
		err = errno;
		GP_DEBUG(1, "Failed to open file '%s'", path);
		goto err0;
	}

	if (fread(buf, sizeof(buf), 1, f) < 1) {
		GP_DEBUG(1, "Failed to read start of the file '%s'", path);
		err = EIO;
		goto err1;
	}

	fclose(f);

	ret = GP_MatchSignature(buf);

	if (ret == NULL)
		errno = ENOSYS;

	return ret;
err1:
	fclose(f);
err0:
	errno = err;
	return NULL;
}

GP_Context *GP_ReadImage(GP_IO *io, GP_ProgressCallback *callback)
{
	char buf[32];
	off_t start;
	const GP_Loader *loader;

	start = GP_IOTell(io);
	if (start == (off_t)-1) {
		GP_DEBUG(1, "Failed to get IO stream offset: %s",
		         strerror(errno));
		return NULL;
	}

	if (GP_IOFill(io, buf, sizeof(buf))) {
		GP_DEBUG(1, "Failed to read first 32 bytes: %s",
		         strerror(errno));
		return NULL;
	}

	if (GP_IOSeek(io, start, GP_IO_SEEK_SET) != start) {
		GP_DEBUG(1, "Failed to seek at the start of the stream: %s",
		         strerror(errno));
		return NULL;
	}

	loader = GP_MatchSignature(buf);

	if (!loader) {
		GP_DEBUG(1, "Failed to find a loader by signature for"
		            "(%x (%c) %x (%c)...)",
			    buf[0], isprint(buf[0]) ? buf[0] : ' ',
			    buf[1], isprint(buf[1]) ? buf[1] : ' ');
		errno = ENOSYS;
		return NULL;
	}

	if (!loader->Read) {
		GP_DEBUG(1, "Loader for '%s' does not support reading",
		         loader->fmt_name);
		errno = ENOSYS;
		return NULL;
	}

	return loader->Read(io, callback);
}

GP_Context *GP_LoadImage(const char *src_path, GP_ProgressCallback *callback)
{
	int err;
	struct stat st;

	if (access(src_path, R_OK)) {
		err = errno;
		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		errno = err;
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

int GP_LoadMetaData(const char *src_path, GP_MetaData *data)
{
	const char *ext;
	int err;

	if (access(src_path, R_OK)) {
		err = errno;
		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		errno = err;
		return 1;
	}

	ext = get_ext(src_path);

	if (ext == NULL)
		goto out;

	if (!strcasecmp(ext, "jpg") || !strcasecmp(ext, "jpeg"))
		return GP_LoadJPGMetaData(src_path, data);

	if (!strcasecmp(ext, "png"))
		return GP_LoadPNGMetaData(src_path, data);

out:
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
