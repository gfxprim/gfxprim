%% extends "filter.point.c.t"

%% block descr
Contrast filters -- Multiply color channel(s) by a fixed float value.
%% endblock

%% block body

{{ filter_include() }}

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
