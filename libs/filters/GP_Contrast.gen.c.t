@ include source.t
/*
 * Contrast Point filter
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_Clamp.h"

@ include point_filter.t
@
@ def filter_op_contrast(val, val_max):
GP_CLAMP_GENERIC({{ val }} * p + 0.5, 0, {{ val_max }})
@ end

{@ filter_point('Contrast', filter_op_contrast, 'float p', 'p') @}
