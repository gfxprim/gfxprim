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
Point filters -- General point filter.
%% endblock

%% block body

{{ filter_point_include() }}

typedef uint32_t (*func)(uint32_t, uint8_t, GP_FilterParam *priv);

%% macro filter_op(chan_name, chan_size)
{{ chan_name }} = {{ chan_name }}_func({{ chan_name }}, {{ chan_size }}, {{ chan_name }}_priv);
%% endmacro

/*
 * Generated point filters for pixels with several channels.
 */
%% call(pt) filter_point_per_channel('Point', 'GP_FilterParam filter_callbacks[], GP_FilterParam priv[]', filter_op)
{{ filter_params(pt, 'filter_callbacks', 'func ', '_func', 'ptr') }}
{{ filter_params_raw(pt, 'priv', '_priv') }}
%% endcall

/*
 * Generated point filters for pixels with one channel.
 */
%% call(ps) filter_point_per_bpp('Point', 'GP_FilterParam filter_callbacks[], GP_FilterParam priv[]', filter_op)
{{ filter_param(ps, 'filter_callbacks', 'func ', '_func', 'ptr') }}
{{ filter_param_raw(ps, 'priv', '_priv') }}
%% endcall

{{ filter_functions('Point', 'GP_FilterParam filter_callbacks[], GP_FilterParam priv[]', 'filter_callbacks, priv') }}

%% endblock body
