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

void GP_LineAA(GP_Context *context, GP_Coord x1, GP_Coord y1,
               GP_Coord x2, GP_Coord y2, GP_Pixel pixel)
{
	GP_Coord dx = x2 - x1;
	GP_Coord dy = y2 - y1;

	if (dy == 0) {
		//TODO!!!
		//GP_HLine_Raw(context, GP_FP_ROUND_TO_INT(x1), GP_FP_ROUND_TO_INT(x2),
		//                      GP_FP_ROUND_TO_INT(y1), pixel);
		return;
	}

	if (dx == 0) {
		//TODO!!!
		//GP_VLine(context, GP_FP_ROUND_TO_INT(x1), GP_FP_ROUND_TO_INT(y1),
		//                  GP_FP_ROUND_TO_INT(y2), pixel);
		return;
	}

	if (GP_ABS(dx) < GP_ABS(dy)) {
		GP_SWAP(x1, y1);
		GP_SWAP(x2, y2);
		GP_SWAP(dx, dy);
		//TODO
		return;
	}
	
	if (x2 < x1) {
		GP_SWAP(x1, x2);
		GP_SWAP(y1, y2);
	}

	GP_Coord grad = GP_FP_DIV(dy, dx);

	GP_Coord xend, yend, xgap, xpx1, ypx1, xpx2, ypx2;

	xend = GP_FP_ROUND(x1);
	yend = y1 + GP_FP_MUL(grad, xend - x1);
	xgap = GP_FP_RFRAC(x1 + GP_FP_1_2);
	xpx1 = GP_FP_TO_INT(xend);
	ypx1 = GP_FP_TO_INT(yend);

	GP_MixPixel_Raw_Clipped(context, xpx1, ypx1, pixel, FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(yend), xgap)));
	GP_MixPixel_Raw_Clipped(context, xpx1, ypx1 + 1, pixel, FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(yend), xgap)));

	GP_Coord intery = yend + grad;

	xend = GP_FP_ROUND(x2);
	yend = y2 + GP_FP_MUL(grad, xend - x2);
	xgap = GP_FP_FRAC(x2 + GP_FP_1_2);
	xpx2 = GP_FP_TO_INT(xend);
	ypx2 = GP_FP_TO_INT(yend);

	GP_MixPixel_Raw_Clipped(context, xpx2, ypx2, pixel, FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(yend), xgap)));
	GP_MixPixel_Raw_Clipped(context, xpx2, ypx2 + 1, pixel, FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(yend), xgap)));

	GP_Coord x;

	for (x = xpx1 + 1; x < xpx2; x++) {
		GP_MixPixel_Raw_Clipped(context, x, GP_FP_TO_INT(intery), pixel, FP_TO_PERC(GP_FP_RFRAC(intery)));
		GP_MixPixel_Raw_Clipped(context, x, GP_FP_TO_INT(intery) + 1, pixel, FP_TO_PERC(GP_FP_FRAC(intery)));

		intery += grad;
	}
}


%% endblock body
