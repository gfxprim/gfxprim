%% extends "base.c.t"

%% block descr
Vertical Mirror alogorithm
%% endblock

%% block body

#include "core/GP_GetPutPixel.h"
#include "core/GP_Debug.h"
#include "GP_Rotate.h"

%% for ps in pixelsizes
void GP_MirrorV_Raw_{{ ps.suffix }}(const GP_Context *src, GP_Context *dst,
                                    GP_ProgressCallback *callback)
{
	uint32_t x, y;
	GP_Pixel tmp;

	GP_DEBUG(1, "Mirroring image vertically %ux%u", src->w, src->h);
	
	for (x = 0; x < src->w/2; x++) {
		uint32_t xm = src->w - x - 1;
		for (y = 0; y < src->h; y++) {
			tmp = GP_GetPixel_Raw_{{ ps.suffix }}(src, x, y);
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, x, y, GP_GetPixel_Raw_{{ ps.suffix }}(src, xm, y));
			GP_PutPixel_Raw_{{ ps.suffix }}(dst, xm, y, tmp);
		}

		if (callback != NULL && x % 100 == 0)
			GP_ProgressCallbackReport(callback, 200.00 * x / src->w);
	}
	
	GP_ProgressCallbackDone(callback);
}

%% endfor

void GP_FilterMirrorV_Raw(const GP_Context *src, GP_Context *dst,
                          GP_ProgressCallback *callback)
{
	GP_FN_PER_BPP_CONTEXT(GP_MirrorV_Raw, src, src, dst, callback);
}
%% endblock body
