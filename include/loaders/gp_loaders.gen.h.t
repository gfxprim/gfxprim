@ include header.t
/*
 * Inline functions for loading and saving various pixmap formats.
 *
 * Copyright (C) 2017 Cyril Hrubis <metan@ucw.cz>
 */
#include <loaders/gp_loader.h>

@ def reader(fmt):
/*** Function prototypes for {{fmt}} ***/

extern const gp_loader gp_{{fmt}};

int gp_match_{{fmt}}(const void *buf);

int gp_read_{{fmt}}_ex(gp_io *io, gp_pixmap **img, gp_storage *storage,
                   gp_progress_cb *callback);

static inline gp_pixmap *gp_load_{{fmt}}(const char *src_path,
                                     gp_progress_cb *callback)
{
        return gp_loader_load_image(&gp_{{fmt}}, src_path, callback);
}

static inline gp_pixmap *gp_read_{{fmt}}(gp_io *io, gp_progress_cb *callback)
{
        return gp_loader_read_image(&gp_{{fmt}}, io, callback);
}
@ end

@ for fmt in ['bmp', 'jpg', 'png', 'tiff', 'pbm', 'pgm', 'ppm', 'pnm']:
{@ reader(fmt) @}

int gp_write_{{fmt}}(const gp_pixmap *src, gp_io *io,
                 gp_progress_cb *callback);

static inline int gp_save_{{fmt}}(const gp_pixmap *src, const char *dst_path,
                              gp_progress_cb *callback)
{
	return gp_loader_save_image(&gp_{{fmt}}, src, dst_path, callback);
}

@ for fmt in ['jp2', 'pcx', 'gif', 'psp', 'psd']:
{@ reader(fmt) @}
