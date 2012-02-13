%% extends "base.c.t"

{% block descr %}Anti Aliased Horizontal Line{% endblock %}

%% block body

#include "core/GP_Context.h"
#include "core/GP_MixPixels.h"
#include "core/GP_FixedPoint.h"
#include "core/GP_GammaCorrection.h"

#define FP_TO_PERC(a) (GP_FP_ROUND_TO_INT((a) * 255))

void GP_HLineAA_Raw(GP_Context *context, GP_Coord x0, GP_Coord x1,
                    GP_Coord y, GP_Pixel pixel)
{
	if (x1 < x0)
		GP_SWAP(x1, x0);
	
	GP_Coord int_x0 = GP_FP_TO_INT(x0);
	GP_Coord int_x1 = GP_FP_TO_INT(x1);
	GP_Coord int_y  = GP_FP_TO_INT(y);

	/* Line is shorter than two pixels */
	if (int_x0 == int_x1) {
		//TODO		
		return;
	}

	/* Draw the starting and ending pixel */
	uint8_t perc;
	
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(y), GP_FP_RFRAC(x0)));
	GP_MixPixel_Raw_Clipped(context, int_x0, int_y, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(y), GP_FP_RFRAC(x0)));
	GP_MixPixel_Raw_Clipped(context, int_x0, int_y+1, pixel, perc); 
	
	
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(y), GP_FP_FRAC(x1)));
	GP_MixPixel_Raw_Clipped(context, int_x1, int_y, pixel, perc);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(y), GP_FP_FRAC(x1)));
	GP_MixPixel_Raw_Clipped(context, int_x1, int_y+1, pixel, perc); 

	/* Draw the middle pixels */
	uint8_t up = FP_TO_PERC(GP_FP_RFRAC(y));
	uint8_t lp = FP_TO_PERC(GP_FP_FRAC(y));

	GP_Coord x;
	
	for (x = int_x0 + 1; x < int_x1; x++) {
		GP_MixPixel_Raw_Clipped(context, x, int_y, pixel, up);
		GP_MixPixel_Raw_Clipped(context, x, int_y+1, pixel, lp);
	}
}

%% endblock body
