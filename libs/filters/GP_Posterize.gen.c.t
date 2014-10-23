@ include source.t
/*
 * Posterize Point filter
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

@ include point_filter.t
@ def filter_op_posterize(val, val_max):
(({{ val }} + ({{ val_max }} / steps)/2) / ({{ val_max }} / steps)) * ({{ val_max }} / steps);
@ end

{@ filter_point('Posterize', filter_op_posterize, 'unsigned int steps', 'steps') @}
