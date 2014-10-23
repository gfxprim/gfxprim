@ include source.t
/*
 * Difference filter -- Symetric difference of two bitmaps.
 *
 * Copyright (C) 2012-2014 Cyril Hrubis <metan@ucw.cz>
 */

@ include arithmetic_filter.t
@
@ def filter_op(chan_name, chan_size):
{{ chan_name }} = GP_ABS({{ chan_name }}_A - {{ chan_name }}_B);
@ end
@
{@ filter_arithmetic('Difference', filter_op) @}
