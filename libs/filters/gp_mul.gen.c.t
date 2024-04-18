@ include source.t
/*
 * Multiplies two images.
 *
 * Copyright (C) 2012-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/gp_clamp.h>

@ include arithmetic_filter.t
@
@ def filter_op(chan_name, chan_max):
{{ chan_name }} = ({{ chan_name }}_A * {{ chan_name }}_B + {{ chan_max }}/2) / {{ chan_max }};
GP_CLAMP({{ chan_name }}, 0, (int32_t){{ chan_max }});
@ end
@
{@ filter_arithmetic('mul', filter_op) @}

