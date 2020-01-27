@ include source.t
/*
 * Histogram filter -- Compute image histogram
 *
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */

#include <string.h>
#include <errno.h>

#include <core/gp_pixmap.h>
#include <core/gp_pixel.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_debug.h>
#include <filters/GP_Filter.h>
#include <filters/GP_Stats.h>

@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
static int histogram_{{ pt.name }}(gp_histogram *self, const gp_pixmap *src, gp_progress_cb *callback)
{
	if (self->pixel_type != src->pixel_type) {
		GP_WARN("Histogram (%s) and pixmap (%s) pixel type must match",
		        gp_pixel_type_name(self->pixel_type),
			gp_pixel_type_name(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

@         for c in pt.chanslist:
	gp_histogram_channel *chan_{{ c.name }} = self->channels[{{ c.idx }}];
@         end

	uint32_t x, y;

	for (y = 0; y < src->h; y++) {
		for (x = 0; x < src->w; x++) {
			gp_pixel pix = gp_getpixel_raw_{{ pt.pixelsize.suffix }}(src, x, y);
@         for c in pt.chanslist:
			int32_t {{ c.name }} = GP_PIXEL_GET_{{ c.name }}_{{ pt.name }}(pix);
@         end

@         for c in pt.chanslist:
			chan_{{ c.name }}->hist[{{ c.name }}]++;
@         end
		}

		if (gp_progress_cb_report(callback, y, src->h, src->w))
			return 1;
	}

	gp_progress_cb_done(callback);
	return 0;
}

@ end
@
int gp_filter_histogram(gp_histogram *self, const gp_pixmap *src,
                       gp_progress_cb *callback)
{
	unsigned int i, j;
	int ret;

	GP_DEBUG(1, "Running Histogram filter");

	for (i = 0; i < gp_pixel_channel_count(self->pixel_type); i++) {
		gp_histogram_channel *chan = self->channels[i];
		memset(chan->hist, 0, sizeof(uint32_t) * chan->len);
	}

	switch (src->pixel_type) {
@ for pt in pixeltypes:
@     if not pt.is_unknown() and not pt.is_palette():
	case GP_PIXEL_{{ pt.name }}:
		ret = histogram_{{ pt.name }}(self, src, callback);
	break;
@ end
	default:
		errno = ENOSYS;
		return 1;
	break;
	}

	if (ret)
		return ret;

	for (i = 0; i < gp_pixel_channel_count(self->pixel_type); i++) {
		gp_histogram_channel *chan = self->channels[i];

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
