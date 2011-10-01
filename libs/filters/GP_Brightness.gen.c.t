%% extends "filter.point.c.t"

%% block descr
Brightness filters -- Increments all color channels by a fixed value.
%% endblock

%% block body

{{ filter_include() }}

%% call(ps) filter_per_pixel_size('Brightness', 'int32_t inc')
pix = pix + inc;
{{ filter_clamp_val('pix', ps.size) }}
%% endcall

%% call(chan) filter_per_pixel_type('Brightness', 'int32_t inc')
{{ chan[0] }} = {{ chan[0] }} + inc;
{{ filter_clamp_val(chan[0], chan[2]) }} 
%% endcall

{{ filter_functions('Brightness', 'int32_t inc', 'inc', "inc=%i") }}

%% endblock body
