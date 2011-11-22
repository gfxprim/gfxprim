%% extends "filter.point.c.t"

%% block descr
Brightness filters -- Increments color channel(s) by a fixed value.
%% endblock

%% block body

{{ filter_include() }}

%% macro filter_op(chann_name, chann_size)
{{ chann_name }} = {{ chann_name }} + {{ chann_name }}_i;
{{ filter_clamp_val(chann_name, chann_size) }}
%% endmacro

/*
 * Generated brightness filters.
 */
%% call(pt) filter_point_per_channel('Brightness', 'GP_FilterParam params[]', filter_op)
{{ filter_params(pt, 'params', 'int32_t', 'i') }}
%% endcall

/*
 * Generated constrast filters for pixels with one channel.
 */
%% call(ps) filter_point_per_bpp('Brightness', 'GP_FilterParam params[]', filter_op)
{{ filter_param(ps, 'params', 'int32_t', 'i') }}
%% endcall

{{ filter_functions('Brightness', 'GP_FilterParam params[]', 'params') }}

%% endblock body
