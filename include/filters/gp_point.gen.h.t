@ include header.t
/*
 * Point filter prototypes.
 *
 * Copyright (C) 2018 Cyril Hrubis <metan@ucw.cz>
 */
#include <filters/gp_filter.h>

@ def filter(name, args, params):
/*** Function prototypes for {{name}} filter ***/

int gp_filter_{{name}}_ex(const gp_pixmap *src,
                          gp_coord x_src, gp_coord y_src,
			  gp_size w_src, gp_size h_src,
                          gp_pixmap *dst,
                          gp_coord x_dst, gp_coord y_dst,
                          {{maybe_opts_r(args)}} gp_progress_cb *callback);

gp_pixmap *gp_filter_{{name}}_ex_alloc(const gp_pixmap *src,
                                       gp_coord x_src, gp_coord y_src,
                                       gp_size w_src, gp_size h_src,
                                       {{maybe_opts_r(args)}} gp_progress_cb *callback);

static inline int gp_filter_{{name}}(const gp_pixmap *src, gp_pixmap *dst,
                                     {{maybe_opts_r(args)}}gp_progress_cb *callback)
{
	return gp_filter_{{name}}_ex(src, 0, 0, src->w, src->h,
	                             dst, 0, 0, {{maybe_opts_r(params)}} callback);
}

static inline gp_pixmap *gp_filter_{{name}}_alloc(const gp_pixmap *src,
                                                  {{maybe_opts_r(args)}}gp_progress_cb *callback)
{
	return gp_filter_{{name}}_ex_alloc(src, 0, 0, src->w, src->h,
	                                   {{maybe_opts_r(params)}} callback);
}

@ end

{@ filter('brightness', 'float p', 'p') @}
{@ filter('contrast', 'float p', 'p') @}
{@ filter('brightness_contrast', 'float b, float c', 'b, c') @}
{@ filter('posterize', 'unsigned int steps', 'steps') @}
{@ filter('invert', '', '') @}
