@ def filter_point_ex(op_name, filter_op, fopts):
int gp_filter_{{ op_name }}_ex(const gp_pixmap *const src,
                               gp_coord x_src, gp_coord y_src,
                               gp_size w_src, gp_size h_src,
                               gp_pixmap *dst,
                               gp_coord x_dst, gp_coord y_dst,
                               {{ maybe_opts_r(fopts) }}
                               gp_progress_cb *callback)
{
	const gp_pixel_type_desc *desc;
	gp_filter_tables tables;
	unsigned int i;
	gp_pixel j;
	int ret, err;

	if (gp_filter_tables_init(&tables, src))
		return 1;

	desc = gp_pixel_desc(src->pixel_type);

	for (i = 0; i < desc->numchannels; i++) {
		gp_pixel chan_max = (1 << desc->channels[i].size);
		gp_pixel *table = tables.table[i];

		for (j = 0; j < chan_max; j++)
			table[j] = {@ filter_op('((signed)j)', '((signed)chan_max - 1)') @};
	}

	ret = gp_filter_tables_apply(src, x_src, y_src, w_src, h_src,
	                             dst, x_dst, y_dst, &tables, callback);

	err = errno;
	gp_filter_tables_free(&tables);
	errno = err;

	return ret;
}
@
@ def filter_point_ex_alloc(op_name, fopts, opts):
gp_pixmap *gp_filter_{{ op_name }}_ex_alloc(const gp_pixmap *const src,
                                            gp_coord x_src, gp_coord y_src,
                                            gp_size w_src, gp_size h_src,
                                            {{ maybe_opts_r(fopts) }}
                                            gp_progress_cb *callback)
{
	gp_pixmap *new = gp_pixmap_alloc(w_src, h_src, src->pixel_type);

	if (gp_filter_{{ op_name }}_ex(src, x_src, y_src, w_src, h_src, new, 0, 0,
	                               {{ maybe_opts_r(opts) }}callback)) {
		int err = errno;
		gp_pixmap_free(new);
		errno = err;
		return NULL;
	}

	return new;
}
@
@ def filter_point(op_name, filter_op, fopts="", opts=""):
#include <errno.h>

#include <core/gp_debug.h>

#include <filters/GP_ApplyTables.h>
#include <filters/GP_Point.h>

{@ filter_point_ex(op_name, filter_op, fopts) @}
{@ filter_point_ex_alloc(op_name, fopts, opts) @}
@ end
