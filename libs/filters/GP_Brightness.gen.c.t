@ include source.t
/*
 * Brightness Point filter
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <core/GP_Clamp.h>

@ include point_filter.t
@
@ def filter_op_brightness(val, val_max):
GP_CLAMP_GENERIC({{ val }} + (p * {{ val_max }} + 0.5), 0, {{ val_max }})
@ end

{@ filter_point('brightness', filter_op_brightness, 'float p', 'p') @}
