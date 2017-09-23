@ include source.t
/*
 * Histogram filter -- Compute image histogram
 *
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>

#include <core/GP_Pixmap.h>
#include <core/GP_Pixel.h>
#include <core/GP_GetPutPixel.h>
#include <core/GP_Debug.h>
#include <filters/GP_Filter.h>
#include <filters/GP_Stats.h>

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
static int GP_FilterHistogram_{{ pt.name }}(GP_Histogram *self,
	const GP_Pixmap *src, GP_ProgressCallback *callback)
{
	if (self->pixel_type != src->pixel_type) {
		GP_WARN("Histogram (%s) and pixmap (%s) pixel type must match",
		        GP_PixelTypeName(self->pixel_type),
			GP_PixelTypeName(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

@         for c in pt.chanslist:
	  GP_HistogramChannel *chan_{{ c.name }} = self->channels[{{ c.idx }}];
@         end

	uint32_t x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++) {
			GP_Pixel pix = GP_GetPixel_Raw_{{ pt.pixelsize.suffix }}(src, x, y);
@         for c in pt.chanslist:
			int32_t {{ c.name }} = GP_Pixel_GET_{{ c.name }}_{{ pt.name }}(pix);
@         end

@         for c in pt.chanslist:
			chan_{{ c.name }}->hist[{{ c.name }}]++;
@         end
		}

		if (GP_ProgressCallbackReport(callback, y, src->h, src->w))
			return 1;
	}

	GP_ProgressCallbackDone(callback);
	return 0;
}

@ end
@
int GP_FilterHistogram(GP_Histogram *self, const GP_Pixmap *src,
                       GP_ProgressCallback *callback)
{
	unsigned int i, j;
	int ret;

	GP_DEBUG(1, "Running Histogram filter");

	for (i = 0; i < GP_PixelChannelCount(self->pixel_type); i++) {
		GP_HistogramChannel *chan = self->channels[i];
		printf("CHAN %i %i %p\n", i, chan->len, chan->hist);
		memset(chan->hist, 0, sizeof(uint32_t) * chan->len);
	}

	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		ret = GP_FilterHistogram_{{ pt.name }}(self, src, callback);
	break;
@ end
	default:
		errno = ENOSYS;
		return 1;
	break;
	}

	if (ret)
		return ret;

	for (i = 0; i < GP_PixelChannelCount(self->pixel_type); i++) {
		GP_HistogramChannel *chan = self->channels[i];

		chan->max = chan->hist[0];
		chan->min = chan->hist[0];

		for (j = 1; j < chan->len; j++) {
			if (chan->hist[j] > chan->max)
				chan->max = chan->hist[j];

			if (chan->hist[j] < chan->min)
				chan->min = chan->hist[j];
		}
	}

	return 0;
}
