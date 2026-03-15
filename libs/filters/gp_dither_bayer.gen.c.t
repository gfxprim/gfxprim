@ include source.t
/*
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */
#include <string.h>
#include <errno.h>

#include <core/gp_debug.h>
#include <core/gp_pixel.h>
#include <core/gp_temp_alloc.h>
#include <core/gp_get_put_pixel.h>
#include <core/gp_convert.h>
#include <core/gp_clamp.h>
#include <filters/gp_filter.h>
#include <filters/gp_dither.gen.h>

/**
 * 4x4 Bayer matrix scaled to UINT16_MAX
 */
static const uint32_t bayer_4x4[4][4] = {
    {  3855, 50115, 15420, 61680 },
    { 34695, 19290, 46260, 30870 },
    { 11565, 57825,  7710, 53985 },
    { 42405, 27015, 38550, 23145 }
};

/**
 * 8x8 Bayer matrix scaled to UIN16_MAX
 */
static const uint32_t bayer_8x8[8][8] = {
    {     0, 32896,  8224, 41120,  2056, 34952, 10280, 43176 },
    { 49344, 16448, 57568, 24672, 51400, 18504, 59624, 26728 },
    { 12336, 45232,  4112, 37008, 14392, 47288,  6168, 39064 },
    { 61680, 28784, 53456, 20560, 63736, 30840, 55512, 22616 },
    {  3084, 35980, 11308, 44204,  1028, 33924,  9252, 42148 },
    { 52428, 19532, 60652, 27756, 50372, 17476, 58596, 25700 },
    { 15420, 48316,  7196, 40092, 13364, 46260,  5140, 38036 },
    { 64764, 31868, 56540, 23644, 62708, 29812, 54484, 21588 }
};

@ def fetch_row(src, row, pt, y):
for (x = 0; x < (gp_coord){{ src }}->w; x++) {
@     if pt.is_gray():
@         for c in pt.chanslist:
	{{ row }}_{{ c.name }}[x+1] = gp_pixel_to_G8(gp_getpixel_raw({{ src }}, x, {{ y }}), {{ src }}->pixel_type);
@	  end
@     else:
	gp_pixel tmp_pix = gp_pixel_to_RGB888(gp_getpixel_raw({{ src }}, x, {{ y }}), {{ src }}->pixel_type);
@         for c in pt.chanslist:
	{{ row }}_{{ c.name }}[x+1] = GP_PIXEL_GET_{{ c.name }}_RGB888(tmp_pix);
@	  end
@     end
}
@     for c in pt.chanslist:
{{ row }}_{{ c.name }}[0] = {{ row }}_{{ c.name }}[1];
{{ row }}_{{ c.name }}[{{ src }}->w+1] = {{ row }}_{{ c.name }}[{{ src }}->w];
@     end
@ end
@
@ def gen_dither(name, fname, bsize, sharpening):
@     for pt in pixeltypes:
@         if pt.is_gray() or pt.is_rgb() and not pt.is_alpha():
/*
 * {{ name }} to {{ pt.name }}
 */
static int {{ fname }}_to_{{ pt.name }}_raw(const gp_pixmap *src,
                                                gp_pixmap *dst,
                                                gp_progress_cb *callback)
{
	gp_coord x, y;

	GP_DEBUG(1, "{{ name }} %s to %s %ux%u",
	            gp_pixel_type_name(src->pixel_type),
	            gp_pixel_type_name(GP_PIXEL_{{ pt.name }}),
		    src->w, src->h);
@             if sharpening:

	gp_temp_alloc_create(temp, sizeof(int16_t) * 3 * (src->w + 2) * {{ pt.chan_cnt }});

@                 for c in pt.chanslist:
	int16_t *row0_{{ c.name }} = gp_temp_alloc_get(temp, sizeof(int16_t) * (src->w + 2));
	int16_t *row1_{{ c.name }} = gp_temp_alloc_get(temp, sizeof(int16_t) * (src->w + 2));
	int16_t *row2_{{ c.name }} = gp_temp_alloc_get(temp, sizeof(int16_t) * (src->w + 2));
@                 end
	{@ fetch_row('src', 'row1', pt, 0) @}

	{@ fetch_row('src', 'row2', pt, 0) @}
@             end

@             for c in pt.chanslist:
	const int step_{{ c.name }} = UINT16_MAX / {{ c.max }};
@             end

	for (y = 0; y < (gp_coord)src->h; y++) {
@             if sharpening:
		/* Shuffle rows */
		int16_t *tmp_row;
@                 for c in pt.chanslist:
		tmp_row = row0_{{ c.name }};
		row0_{{ c.name }} = row1_{{ c.name }};
		row1_{{ c.name }} = row2_{{ c.name }};
		row2_{{ c.name }} = tmp_row;
@                 end
		/* Fetch new row */
		gp_coord row_y = GP_MIN(y+1, (gp_coord)src->h-1);

		{@ fetch_row('src', 'row2', pt, 'row_y') @}
@             end
		for (x = 0; x < (gp_coord)src->w; x++) {
@             if not sharpening:
@                 if pt.is_rgb():
			gp_pixel pix = gp_pixel_to_RGB888(gp_getpixel_raw(src, x, y), src->pixel_type);
@             end

@             for c in pt.chanslist:
@                 if sharpening:
			int32_t pix_{{ c.name }} = row1_{{ c.name }}[x+1]<<8;

			/* Sharpening */
			int32_t avg_{{ c.name }} =
			3*row0_{{ c.name }}[x] + 6*row0_{{ c.name }}[x+1] + 3*row0_{{ c.name }}[x+2] +
			6*row1_{{ c.name }}[x] - 4*row1_{{ c.name }}[x+1] + 6*row1_{{ c.name }}[x+2] +
			3*row2_{{ c.name }}[x] + 6*row2_{{ c.name }}[x+1] + 3*row2_{{ c.name }}[x+2];

			avg_{{ c.name }} <<= 3;

			pix_{{ c.name }} += (pix_{{ c.name }} - avg_{{ c.name }}) * 4 / (1<<{{ c.size }});

			pix_{{ c.name }} = GP_CLAMP(pix_{{ c.name }}, 0, UINT16_MAX);
@                 else:
@                     if pt.is_gray():
			uint32_t pix_{{ c.name }} = gp_pixel_to_G8(gp_getpixel_raw(src, x, y), src->pixel_type);
@                     else:
			uint32_t pix_{{ c.name }} = GP_PIXEL_GET_{{ c.name }}_RGB888(pix);
@                     end
			pix_{{ c.name }} = pix_{{ c.name }} << 8;
@                 end

			gp_pixel res_{{ c.name }} = pix_{{ c.name }} / step_{{ c.name }};
			gp_pixel rem_{{ c.name }} = pix_{{ c.name }} % step_{{ c.name }};

@                 if bsize == 4:
			uint32_t thresh_{{ c.name }} = (bayer_4x4[y & 3][x & 3] * step_{{ c.name}}) / UINT16_MAX;
@                 else:
			uint32_t thresh_{{ c.name }} = (bayer_8x8[y & 7][x & 7] * step_{{ c.name}}) / UINT16_MAX;
@                 endif

			if (rem_{{ c.name }} > thresh_{{ c.name }} && res_{{ c.name }} < {{ c.max }})
				res_{{ c.name }}++;
@             end

@             if pt.is_gray():
			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, x, y, res_{{ c.name }});
@             else:
			gp_pixel res = GP_PIXEL_CREATE_{{ pt.name }}({{ arr_to_params(pt.chan_names, 'res_') }});

			gp_putpixel_raw_{{ pt.pixelpack.suffix }}(dst, x, y, res);
@             end
		}

		if (gp_progress_cb_report(callback, y, src->h, src->w)) {
@             if sharpening:
			gp_temp_alloc_free(temp);
@             end
			return 1;
		}
	}

	gp_progress_cb_done(callback);
@             if sharpening:
	gp_temp_alloc_free(temp);
@             end
	return 0;
}

@     end
static int {{ fname }}(const gp_pixmap *src, gp_pixmap *dst,
                            gp_progress_cb *callback)
{
	if (gp_pixel_has_flags(src->pixel_type, GP_PIXEL_IS_PALETTE)) {
		GP_DEBUG(1, "Unsupported source pixel type %s",
		         gp_pixel_type_name(src->pixel_type));
		errno = EINVAL;
		return 1;
	}

	switch (dst->pixel_type) {
@     for pt in pixeltypes:
@         if pt.is_gray() or pt.is_rgb() and not pt.is_alpha():
	case GP_PIXEL_{{ pt.name }}:
		return {{ fname }}_to_{{ pt.name }}_raw(src, dst, callback);
@     end
	default:
		errno = EINVAL;
		return 1;
	}
}

int gp_filter_{{ fname }}(const gp_pixmap *src, gp_pixmap *dst,
                              gp_progress_cb *callback)
{
	GP_CHECK(src->w <= dst->w);
	GP_CHECK(src->h <= dst->h);
	return {{ fname }}(src, dst, callback);
}

@ end
@
{@ gen_dither("Bayer 4x4", "bayer_4", 4, False) @}
{@ gen_dither("Sharpened Bayer 4x4", "sharpened_bayer_4", 4, True) @}
{@ gen_dither("Bayer 8x8", "bayer_8", 8, False) @}
{@ gen_dither("Sharpened Bayer 8x8", "sharpened_bayer_8", 8, True) @}
