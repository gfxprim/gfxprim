@ include source.t
/*
 * Addition of two bitmaps.
 *
 * Copyright (C) 2012-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/GP_Clamp.h>

@ include arithmetic_filter.t
@
@ def filter_op(chan_name, chan_size):
{{ chan_name }} = {{ chan_name }}_A + {{ chan_name }}_B;
GP_CLAMP_GENERIC({{ chan_name }}, 0, {{ 2 ** chan_size - 1 }});
@ end
@
{@ filter_arithmetic('add', filter_op) @}
