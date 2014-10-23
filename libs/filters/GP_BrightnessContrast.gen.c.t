@ include source.t
/*
 * Brightness and Contrast Point filter
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include "core/GP_Clamp.h"

@ include point_filter.t
@ def filter_op_brightness_contrast(val, val_max):
GP_CLAMP_GENERIC(c * {{ val }} + b * {{ val_max }} + 0.5, 0, {{ val_max }})
@ end

{@ filter_point('BrightnessContrast', filter_op_brightness_contrast, 'float b, float c', 'b, c') @}
