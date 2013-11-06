%% extends "filter.c.t"

%% macro filter_point_include()
#include <errno.h>

#include "core/GP_Debug.h"

#include "filters/GP_ApplyTables.h"
#include "filters/GP_Point.h"
%% endmacro

%% macro filter_point_ex(op_name, filter_op, fopts)
int GP_Filter{{ op_name }}Ex(const GP_FilterArea *const area{{ maybe_opts_l(fopts) }},
                          GP_ProgressCallback *callback)
{
	const GP_PixelTypeDescription *desc;
	GP_FilterTables tables;
	unsigned int i;
	GP_Pixel j;
	int ret, err;

	if (GP_FilterTablesInit(&tables, area->src))
		return 1;

	desc = GP_PixelTypeDesc(area->src->pixel_type);

	for (i = 0; i < desc->numchannels; i++) {
		GP_Pixel chan_max = (1 << desc->channels[i].size);
		GP_Pixel *table = tables.table[i];

		for (j = 0; j < chan_max; j++)
			table[j] = {{ filter_op('((signed)j)', '((signed)chan_max - 1)') }};
	}

	ret = GP_FilterTablesApply(area, &tables, callback);

	err = errno;
	GP_FilterTablesFree(&tables);
	errno = err;

	return ret;
}
%% endmacro

%% macro filter_point_ex_alloc(op_name, fopts, opts)
GP_Context *GP_Filter{{ op_name }}ExAlloc(const GP_FilterArea *const area{{ maybe_opts_l(fopts) }},
					  GP_ProgressCallback *callback)
{
	GP_FilterArea carea = *area;
	GP_Context *new = GP_ContextAlloc(area->src_w, area->src_h,
	                                  area->src->pixel_type);

	/* Clear these just to be sure */
	carea.dst_x = 0;
	carea.dst_y = 0;
	carea.dst = new;

	if (GP_Filter{{ op_name }}Ex(&carea, {{ maybe_opts_r(opts) }}callback)) {
		int err = errno;
		GP_ContextFree(new);
		errno = err;
		return NULL;
	}

	return new;
}
%% endmacro

%% macro filter_point(op_name, filter_op, fopts="", opts="")
{{ filter_point_include() }}
{{ filter_point_ex(op_name, filter_op, fopts) }}
{{ filter_point_ex_alloc(op_name, fopts, opts) }}
%% endmacro
