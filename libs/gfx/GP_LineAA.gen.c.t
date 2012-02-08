%% extends "base.c.t"

{% block descr %}Anti Aliased Line{% endblock %}

%% block body

#include "core/GP_Context.h"
#include "core/GP_MixPixels.h"
#include "core/GP_FixedPoint.h"
#include "core/GP_GammaCorrection.h"

#include "gfx/GP_HLine.h"
#include "gfx/GP_VLine.h"

#define FP_TO_PERC(a) (GP_FP_ROUND_TO_INT((a) * 255))

void GP_LineAA_Raw(GP_Context *context, GP_Coord x0, GP_Coord y0,
                   GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	int64_t dx = x1 - x0;
	int64_t dy = y1 - y0;

	if (dy == 0) {
		//TODO!!!
		GP_HLine_Raw(context, GP_FP_ROUND_TO_INT(x0), GP_FP_ROUND_TO_INT(x1),
		                      GP_FP_ROUND_TO_INT(y0), pixel);
		return;
	}

	if (dx == 0) {
		//TODO!!!
		GP_VLine(context, GP_FP_ROUND_TO_INT(x0), GP_FP_ROUND_TO_INT(y0),
		                  GP_FP_ROUND_TO_INT(y1), pixel);
		return;
	}

	if (GP_ABS(dx) < GP_ABS(dy)) {
		//TODO
		return;
	}
	
	if (x1 < x0) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	GP_Coord xend, yend, xgap, xpx0, ypx0, xpx1, ypx1;
	uint8_t perc;

	xend = GP_FP_ROUND(x1);
	yend = y1 + GP_FP_DIV(GP_FP_MUL(dy, xend - x1), dx);
	xgap = GP_FP_FRAC(x1 + GP_FP_1_2);
	xpx1 = GP_FP_TO_INT(xend);
	ypx1 = GP_FP_TO_INT(yend);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(yend), xgap));
	GP_MixPixel_Raw_Clipped(context, xpx1, ypx1, pixel, perc);
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(yend), xgap));
	GP_MixPixel_Raw_Clipped(context, xpx1, ypx1+1, pixel, perc);

	xend = GP_FP_ROUND(x0);
	yend = y0 + GP_FP_DIV(GP_FP_MUL(dy, xend - x0), dx);
	xgap = GP_FP_RFRAC(x0 + GP_FP_1_2);
	xpx0 = GP_FP_TO_INT(xend);
	ypx0 = GP_FP_TO_INT(yend);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(yend), xgap));
	GP_MixPixel_Raw_Clipped(context, xpx0, ypx0, pixel, perc);
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(yend), xgap));
	GP_MixPixel_Raw_Clipped(context, xpx0, ypx0+1, pixel, perc);

	GP_Coord x;
	GP_Coord intery;

	for (x = xpx0 + 1; x < xpx1; x++) {
		intery = yend + GP_FP_DIV((x - xpx0) * dy, dx);
		GP_MixPixel_Raw_Clipped(context, x, GP_FP_TO_INT(intery), pixel, FP_TO_PERC(GP_FP_RFRAC(intery)));
		GP_MixPixel_Raw_Clipped(context, x, GP_FP_TO_INT(intery) + 1, pixel, FP_TO_PERC(GP_FP_FRAC(intery)));
	}
}

%% endblock body
