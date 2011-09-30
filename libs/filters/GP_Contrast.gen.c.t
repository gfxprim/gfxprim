%% extends "filter.point.c.t"

%% block descr
Contrast filters -- Multiply all color channels by a fixed value.
%% endblock

%% block body
#include <GP_Context.h>
#include <GP_Pixel.h>
#include <GP_GetPutPixel.h>

%% call(ps) filter_per_pixel_size('Contrast', 'float mul')
pix = 1.00 * pix * mul;
{{ filter_clamp_val('pix', ps.size) }}
%% endcall

%% call(chan) filter_per_pixel_type('Contrast', 'float mul')
{{ chan[0] }} = mul * {{ chan[0] }} + 0.5;
{{ filter_clamp_val(chan[0], chan[2]) }} 
%% endcall

{{ filter_functions('Contrast', 'float mul', 'mul') }}

%% endblock body
