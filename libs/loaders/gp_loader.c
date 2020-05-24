// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

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

#include <core/gp_debug.h>

#include <loaders/gp_loaders.h>
#include <loaders/gp_loader.h>

#define MAX_LOADERS 64

static const gp_loader *loaders[MAX_LOADERS] = {
	&gp_jpg,
	&gp_png,
	&gp_webp,
	&gp_tiff,
	&gp_gif,
	&gp_bmp,
	&gp_pbm,
	&gp_pgm,
	&gp_ppm,
	&gp_pnm,
	&gp_jp2,
	&gp_pcx,
	&gp_psp,
	&gp_psd,
};

static unsigned int get_last_loader(void)
{
	unsigned int i;

	for (i = 0; i < MAX_LOADERS; i++) {
		if (!loaders[i])
			return i ? i - 1 : 0;
	}

	return i - 1;
}

int gp_loader_register(const gp_loader *self)
{
	unsigned int i;

	GP_DEBUG(1, "Registering loader for '%s'", self->fmt_name);

	/* We have to keep the last terminating NULL */
	for (i = 0; i < MAX_LOADERS - 2; i++) {
		if (loaders[i] == self) {
			GP_DEBUG(1, "Loader '%s' allready registered",
			         self->fmt_name);
			errno = EEXIST;
			return 1;
		}

		if (!loaders[i])
			break;
	}

	if (loaders[i]) {
		GP_DEBUG(1, "Loaders table is full");
		errno = ENOSPC;
		return 1;
	}

	loaders[i] = self;

	return 0;
}

void gp_loader_unregister(const gp_loader *self)
{
	unsigned int i, last = get_last_loader();

	if (self == NULL)
		return;

	GP_DEBUG(1, "Unregistering loader for '%s'", self->fmt_name);

	for (i = 0; loaders[i]; i++) {
		if (loaders[i] == self) {
			loaders[i] = loaders[last];
			loaders[last] = NULL;
			return;
		}
	}

	GP_WARN("Loader '%s' (%p) wasn't registered", self->fmt_name, self);
}

void gp_loaders_lists(void)
{
	unsigned int i, j;

	for (i = 0; loaders[i]; i++) {
		printf("Format: %s\n", loaders[i]->fmt_name);
		printf("Read:\t%s\n", loaders[i]->Read ? "Yes" : "No");
		printf("Write:\t%s\n", loaders[i]->Write ? "Yes" : "No");
		if (loaders[i]->save_ptypes) {
			printf("Write Pixel Types: ");
			for (j = 0; loaders[i]->save_ptypes[j]; j++) {
				gp_pixel_type ptype = loaders[i]->save_ptypes[j];
				printf("%s ", gp_pixel_type_name(ptype));
			}
			printf("\n");
		}
		printf("Match:\t%s\n", loaders[i]->Match ? "Yes" : "No");
		printf("Extensions: ");
		for (j = 0; loaders[i]->extensions[j]; j++)
			printf("%s ", loaders[i]->extensions[j]);
		printf("\n");

		if (loaders[i+1] != NULL)
			printf("\n");
	}
}

static const gp_loader *loader_by_extension(const char *ext)
{
	unsigned int i, j;

	for (i = 0; loaders[i]; i++) {
		for (j = 0; loaders[i]->extensions[j] != NULL; j++) {
			if (!strcasecmp(ext, loaders[i]->extensions[j])) {
				GP_DEBUG(1, "Found loader '%s'",
				         loaders[i]->fmt_name);
				return loaders[i];
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

const gp_loader *gp_loader_by_filename(const char *path)
{
	const char *ext = get_ext(path);

	if (ext == NULL)
		return NULL;

	GP_DEBUG(1, "Loading file by filename extension '%s'", ext);

	return loader_by_extension(ext);
}

static const gp_loader *loader_by_signature(const char *path)
{
	uint8_t buf[32];
	FILE *f;
	int err;

	const gp_loader *ret;

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

	ret = gp_loader_by_signature(buf);

	if (ret == NULL)
		errno = ENOSYS;

	return ret;
err1:
	fclose(f);
err0:
	errno = err;
	return NULL;
}

gp_pixmap *gp_read_image(gp_io *io, gp_progress_cb *callback)
{
	gp_pixmap *ret = NULL;

	gp_read_image_ex(io, &ret, NULL, callback);

	return ret;
}

int gp_read_image_ex(gp_io *io, gp_pixmap **img, gp_storage *meta_data,
                     gp_progress_cb *callback)
{
	char buf[32];
	off_t start;
	const gp_loader *loader;

	start = gp_io_tell(io);
	if (start == (off_t)-1) {
		GP_DEBUG(1, "Failed to get IO stream offset: %s",
		         strerror(errno));
		return 1;
	}

	if (gp_io_fill(io, buf, sizeof(buf))) {
		GP_DEBUG(1, "Failed to read first 32 bytes: %s",
		         strerror(errno));
		return 1;
	}

	if (gp_io_seek(io, start, GP_IO_SEEK_SET) != start) {
		GP_DEBUG(1, "Failed to seek at the start of the stream: %s",
		         strerror(errno));
		return 1;
	}

	loader = gp_loader_by_signature(buf);

	if (!loader) {
		GP_DEBUG(1, "Failed to find a loader by signature for"
		            "(%x (%c) %x (%c)...)",
			    buf[0], isprint(buf[0]) ? buf[0] : ' ',
			    buf[1], isprint(buf[1]) ? buf[1] : ' ');
		errno = ENOSYS;
		return 1;
	}

	if (!loader->Read) {
		GP_DEBUG(1, "Loader for '%s' does not support reading",
		         loader->fmt_name);
		errno = ENOSYS;
		return 1;
	}

	return loader->Read(io, img, meta_data, callback);
}

int gp_loader_load_image_ex(const gp_loader *self, const char *src_path,
                            gp_pixmap **img, gp_storage *storage,
                            gp_progress_cb *callback)
{
	gp_io *io;
	int err, ret;

	GP_DEBUG(1, "Loading Image '%s'", src_path);

	if (!self->Read) {
		errno = ENOSYS;
		return ENOSYS;
	}

	io = gp_io_file(src_path, GP_IO_RDONLY);
	if (!io)
		return 1;

	ret = self->Read(io, img, storage, callback);

	err = errno;
	gp_io_close(io);
	errno = err;

	return ret;
}


gp_pixmap *gp_loader_load_image(const gp_loader *self, const char *src_path,
                               gp_progress_cb *callback)
{
	gp_pixmap *ret = NULL;

	gp_loader_load_image_ex(self, src_path, &ret, NULL, callback);

	return ret;
}

gp_pixmap *gp_loader_read_image(const gp_loader *self, gp_io *io,
                               gp_progress_cb *callback)
{
	gp_pixmap *ret = NULL;

	gp_loader_read_image_ex(self, io, &ret, NULL, callback);

	return ret;
}

int gp_loader_read_image_ex(const gp_loader *self, gp_io *io,
                         gp_pixmap **img, gp_storage *data,
                         gp_progress_cb *callback)
{
	GP_DEBUG(1, "Reading image (I/O %p)", io);

	if (!self->Read) {
		errno = ENOSYS;
		return ENOSYS;
	}

	return self->Read(io, img, data, callback);
}

gp_pixmap *gp_load_image(const char *src_path, gp_progress_cb *callback)
{
	gp_pixmap *ret = NULL;

	gp_load_image_ex(src_path, &ret, NULL, callback);

	return ret;
}

int gp_load_image_ex(const char *src_path,
                     gp_pixmap **img, gp_storage *meta_data,
                     gp_progress_cb *callback)
{
	int err;
	struct stat st;

	if (access(src_path, R_OK)) {
		err = errno;
		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		errno = err;
		return 1;
	}

	if (stat(src_path, &st)) {
		GP_WARN("Failed to stat '%s': %s", src_path, strerror(errno));
	} else {
		if (st.st_mode & S_IFDIR) {
			errno = EISDIR;
			return 1;
		}
	}

	const gp_loader *ext_load, *sig_load;

	ext_load = gp_loader_by_filename(src_path);

	if (ext_load) {
		if (!gp_loader_load_image_ex(ext_load, src_path,
		                             img, meta_data, callback))
			return 0;
	}

	/*
	 * Operation was aborted, just here exit.
	 */
	if (errno == ECANCELED)
		return 1;

	sig_load = loader_by_signature(src_path);

	/*
	 * Avoid further work if extension matches the signature but image
	 * couldn't be loaded. Probably unimplemented format or damaged file.
	 */
	if (ext_load == sig_load) {
		GP_WARN("Signature matches extension but file '%s' "
		        "can't be loaded. Unsupported/damaged file?",
			src_path);
		return 1;
	}

	if (ext_load && sig_load) {
		GP_WARN("File '%s': Extension says %s but signature %s",
			src_path, ext_load->fmt_name, sig_load->fmt_name);
	}

	if (sig_load) {
		if (!gp_loader_load_image_ex(sig_load, src_path,
		                          img, meta_data, callback))
			return 0;
	}

	errno = ENOSYS;
	return 1;
}

int gp_load_meta_data(const char *src_path, gp_storage *storage)
{
	const gp_loader *loader;
	struct stat st;
	int err;

	// TODO unify this with gp_load_image()
	if (access(src_path, R_OK)) {
		err = errno;
		GP_DEBUG(1, "Failed to access file '%s' : %s",
		            src_path, strerror(errno));
		errno = err;
		return 1;
	}

	if (stat(src_path, &st)) {
		GP_WARN("Failed to stat '%s': %s", src_path, strerror(errno));
	} else {
		if (st.st_mode & S_IFDIR) {
			errno = EISDIR;
			return 1;
		}
	}

	loader = gp_loader_by_filename(src_path);

	if (loader == NULL)
		goto out;

	return gp_loader_load_image_ex(loader, src_path, NULL, storage, NULL);

out:
	errno = ENOSYS;
	return 1;
}

int gp_loader_save_image(const gp_loader *self, const gp_pixmap *src,
                         const char *dst_path, gp_progress_cb *callback)
{
	gp_io *io;

	GP_DEBUG(1, "Saving image '%s' format %s", dst_path, self->fmt_name);

	if (!self->Write) {
		errno = ENOSYS;
		return 1;
	}

	io = gp_io_file(dst_path, GP_IO_WRONLY);

	if (!io)
		return 1;

	if (self->Write(src, io, callback)) {
		gp_io_close(io);
		unlink(dst_path);
		return 1;
	}

	if (gp_io_close(io)) {
		unlink(dst_path);
		return 1;
	}

	return 0;
}

int gp_save_image(const gp_pixmap *src, const char *dst_path,
                  gp_progress_cb *callback)
{
	const gp_loader *l = gp_loader_by_filename(dst_path);

	if (l == NULL) {
		errno = EINVAL;
		return 1;
	}

	return gp_loader_save_image(l, src, dst_path, callback);
}

const gp_loader *gp_loader_by_signature(const void *buf)
{
	unsigned int i;

	for (i = 0; loaders[i]; i++) {
		if (loaders[i]->Match && loaders[i]->Match(buf) == 1) {
			GP_DEBUG(1, "Found loader '%s'", loaders[i]->fmt_name);
			return loaders[i];
		}
	}

	GP_DEBUG(1, "Loader not found");

	return NULL;
}
