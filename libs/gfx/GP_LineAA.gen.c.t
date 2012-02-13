%% extends "base.c.t"

{% block descr %}Anti Aliased Line{% endblock %}

%% block body

#include "core/GP_Context.h"
#include "core/GP_MixPixels.h"
#include "core/GP_FixedPoint.h"
#include "core/GP_GammaCorrection.h"

#include "gfx/GP_HLineAA.h"
#include "gfx/GP_VLineAA.h"

#define FP_TO_PERC(a) (GP_FP_ROUND_TO_INT((a) * 255))

static inline void line_aa_x(GP_Context *context,
                             GP_Coord x0, GP_Coord y0,
                             GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_Coord xend, yend, xgap, xpx0, ypx0, xpx1, ypx1;
	uint8_t perc;
	
	int64_t dx = x1 - x0;
	int64_t dy = y1 - y0;
	
	if (x1 < x0) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

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
		
		perc = FP_TO_PERC(GP_FP_RFRAC(intery));
		GP_MixPixel_Raw_Clipped(context, x, GP_FP_TO_INT(intery), pixel, perc);
		perc = FP_TO_PERC(GP_FP_FRAC(intery));
		GP_MixPixel_Raw_Clipped(context, x, GP_FP_TO_INT(intery)+1, pixel, perc);
	}
}

static inline void line_aa_y(GP_Context *context,
                             GP_Coord x0, GP_Coord y0,
                             GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	GP_Coord xend, yend, ygap, xpx0, ypx0, xpx1, ypx1;
	uint8_t perc;
	
	int64_t dx = x1 - x0;
	int64_t dy = y1 - y0;
	
	if (y1 < y0) {
		GP_SWAP(x0, x1);
		GP_SWAP(y0, y1);
	}

	yend = GP_FP_ROUND(y1);
	xend = x1 + GP_FP_DIV(GP_FP_MUL(dx, yend - y1), dy);
	ygap = GP_FP_FRAC(y1 + GP_FP_1_2);
	ypx1 = GP_FP_TO_INT(yend);
	xpx1 = GP_FP_TO_INT(xend);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(xend), ygap));
	GP_MixPixel_Raw_Clipped(context, xpx1, ypx1, pixel, perc);
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(xend), ygap));
	GP_MixPixel_Raw_Clipped(context, xpx1, ypx1+1, pixel, perc);

	yend = GP_FP_ROUND(y0);
	xend = x0 + GP_FP_DIV(GP_FP_MUL(dx, yend - y0), dy);
	ygap = GP_FP_RFRAC(y0 + GP_FP_1_2);
	ypx0 = GP_FP_TO_INT(yend);
	xpx0 = GP_FP_TO_INT(xend);

	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_RFRAC(xend), ygap));
	GP_MixPixel_Raw_Clipped(context, xpx0, ypx0, pixel, perc);
	perc = FP_TO_PERC(GP_FP_MUL(GP_FP_FRAC(xend), ygap));
	GP_MixPixel_Raw_Clipped(context, xpx0, ypx0+1, pixel, perc);

	GP_Coord y;
	GP_Coord intery;

	for (y = ypx0 + 1; y < ypx1; y++) {
		intery = xend + GP_FP_DIV((y - ypx0) * dx, dy);
		
		perc = FP_TO_PERC(GP_FP_RFRAC(intery));
		GP_MixPixel_Raw_Clipped(context, GP_FP_TO_INT(intery), y, pixel, perc);
		perc = FP_TO_PERC(GP_FP_FRAC(intery));
		GP_MixPixel_Raw_Clipped(context, GP_FP_TO_INT(intery)+1, y, pixel, perc);
	}
}

void GP_LineAA_Raw(GP_Context *context, GP_Coord x0, GP_Coord y0,
                   GP_Coord x1, GP_Coord y1, GP_Pixel pixel)
{
	int64_t dx = x1 - x0;
	int64_t dy = y1 - y0;

	if (dy == 0) {
		GP_HLineAA_Raw(context, x0, x1, y0, pixel);
		return;
	}

	if (dx == 0) {
		GP_VLineAA_Raw(context, x0, y0, y1, pixel);
		return;
	}

	if (GP_ABS(dx) < GP_ABS(dy))
		line_aa_y(context, x0, y0, x1, y1, pixel);
	else
		line_aa_x(context, x0, y0, x1, y1, pixel);
}

%% endblock body
