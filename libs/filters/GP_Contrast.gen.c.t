%% extends "filter.point.c.t"

%% block descr
Contrast filters -- Multiply color channel(s) by a fixed float value.
%% endblock

%% block body

{{ filter_include() }}

%% macro filter_op(chan_name, chan_size)
{{ chan_name }} = {{ chan_name }} * {{ chan_name }}_f + 0.5;
{{ filter_clamp_val(chan_name, chan_size) }}
%% endmacro

/*
 * Generated contrast filters for pixels with several channels.
 */
%% call(pt) filter_point_per_channel('Contrast', 'GP_FilterParam params[]', filter_op)
{{ filter_params(pt, 'params', 'float', 'f') }}
%% endcall

/*
 * Generated constrast filters for pixels with one channel.
 */
%% call(ps) filter_point_per_bpp('Contrast', 'GP_FilterParam params[]', filter_op)
{{ filter_param(ps, 'params', 'float', 'f') }}
%% endcall

{{ filter_functions('Contrast', 'GP_FilterParam params[]', 'params') }}

%% endblock body
