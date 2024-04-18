@ include source.t
/*
 * Addition of two bitmaps.
 *
 * Copyright (C) 2012-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_clamp.h>

@ include arithmetic_filter.t
@
@ def filter_op(chan_name, chan_max):
{{ chan_name }} = {{ chan_name }}_A + {{ chan_name }}_B;
GP_CLAMP({{ chan_name }}, 0, (int32_t){{ chan_max }});
@ end
@
{@ filter_arithmetic('add', filter_op) @}
