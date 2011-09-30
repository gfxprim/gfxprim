%% extends "filter.point.c.t"

%% block descr
Brightness filters -- Increments all color channels by a fixed value.
%% endblock

%% block body
#include <GP_Context.h>
#include <GP_Pixel.h>
#include <GP_GetPutPixel.h>

%% call(ps) filter_per_pixel_size('Brightness', 'int32_t inc')
pix = pix + inc;
{{ filter_clamp_val('pix', ps.size) }}
%% endcall

%% call(chan) filter_per_pixel_type('Brightness', 'int32_t inc')
{{ chan[0] }} = {{ chan[0] }} + inc;
{{ filter_clamp_val(chan[0], chan[2]) }} 
%% endcall

{{ filter_functions('Brightness', 'int32_t inc', 'inc') }}

%% endblock body
