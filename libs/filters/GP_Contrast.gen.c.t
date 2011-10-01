%% extends "filter.point.c.t"

%% block descr
Contrast filters -- Multiply all color channels by a fixed value.
%% endblock

%% block body

{{ filter_include() }}

%% call(ps) filter_per_pixel_size('Contrast', 'float mul')
pix = 1.00 * pix * mul;
{{ filter_clamp_val('pix', ps.size) }}
%% endcall

%% call(chan) filter_per_pixel_type('Contrast', 'float mul')
{{ chan[0] }} = mul * {{ chan[0] }} + 0.5;
{{ filter_clamp_val(chan[0], chan[2]) }} 
%% endcall

{{ filter_functions('Contrast', 'float mul', 'mul', "mul=%2.3f") }}

%% endblock body
