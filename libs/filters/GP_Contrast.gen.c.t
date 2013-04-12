/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2011      Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

%% extends "filter.point.c.t"

%% block descr
Contrast filters -- Multiply color channel(s) by a fixed float value.
%% endblock

%% block body

{{ filter_point_include() }}

%% macro filter_op(chan_name, chan_size)
{{ chan_name }} = {{ chan_name }} * {{ chan_name }}_mul + 0.5;
{{ filter_clamp_val(chan_name, chan_size) }}
%% endmacro

/*
 * Generated contrast filters for pixels with several channels.
 */
%% call(pt) filter_point_per_channel('Contrast', 'GP_FilterParam muls[]', filter_op)
{{ filter_params(pt, 'muls', 'float ', '_mul', 'f') }}
%% endcall

/*
 * Generated constrast filters for pixels with one channel.
 */
%% call(ps) filter_point_per_bpp('Contrast', 'GP_FilterParam muls[]', filter_op)
{{ filter_param(ps, 'muls', 'float ', '_mul', 'f') }}
%% endcall

{{ filter_functions('Contrast', 'GP_FilterParam muls[]', 'muls') }}

%% endblock body
