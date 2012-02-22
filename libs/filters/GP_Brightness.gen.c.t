%% extends "filter.point.c.t"

%% block descr
Brightness filters -- Increments color channel(s) by a fixed value.
%% endblock

%% block body

{{ filter_point_include() }}

%% macro filter_op(chann_name, chann_size)
{{ chann_name }} = {{ chann_name }} + {{ chann_name }}_inc;
{{ filter_clamp_val(chann_name, chann_size) }}
%% endmacro

/*
 * Generated brightness filters.
 */
%% call(pt) filter_point_per_channel('Brightness', 'GP_FilterParam incs[]', filter_op)
{{ filter_params(pt, 'incs', 'int32_t ', '_inc', 'i') }}
%% endcall

/*
 * Generated constrast filters for pixels with one channel.
 */
%% call(ps) filter_point_per_bpp('Brightness', 'GP_FilterParam incs[]', filter_op)
{{ filter_param(ps, 'incs', 'int32_t ', '_inc', 'i') }}
%% endcall

{{ filter_functions('Brightness', 'GP_FilterParam incs[]', 'incs') }}

%% endblock body
