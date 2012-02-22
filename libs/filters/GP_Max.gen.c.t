%% extends "filter.arithmetic.c.t"

%% block descr
Max filter -- Choose max for each pixel (and for each channel) for two bitmaps.
%% endblock

%% block body

{{ filter_arithmetic_include() }}

%% macro filter_op(chan_name, chan_size)
{{ chan_name }} = GP_MAX({{ chan_name }}_A, {{ chan_name }}_B);
%% endmacro

%% call(pt) filter_arithmetic_per_channel('Max', filter_op)
%% endcall

%% call(ps) filter_arithmetic_per_bpp('Max', filter_op)
%% endcall

{{ filter_arithmetic_functions('Max') }}

%% endblock body
