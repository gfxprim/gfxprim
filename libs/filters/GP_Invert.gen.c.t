@ include source.t
/*
 * Invert Point filter
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

@ include point_filter.t
@
@ def filter_op_invert(val, val_max):
{{ val_max }} - {{ val }}
@ end

{@ filter_point('Invert', filter_op_invert) @}
