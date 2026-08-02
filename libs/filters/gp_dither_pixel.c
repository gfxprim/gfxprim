/*
 * Pixel mapping for ditherings.
 *
 * Copyright (C) 2009-2026 Cyril Hrubis <metan@ucw.cz>
 */
#include <core/gp_pixel.h>


/**
 * Checks if a particular pixel type channel needs dithering.
 */
static inline int channel_needs_dither(const gp_pixel_type_desc *ptd_in,
                                       const gp_pixel_type_desc *ptd_out,
                                       const char chan_name)
{
	uint8_t in_size = gp_pixel_desc_channel_size(ptd_in, chan_name);
	uint8_t out_size = gp_pixel_desc_channel_size(ptd_out, chan_name);

	return in_size > out_size;
}

/**
 * We attempt to map the grayscale pixel type, regardless packing, to a
 * grayscale pixel with default packing and alpha size. Blits with alpha are
 * slow, we do not care about the packing difference.
 *
 * If maping is not found, or not enabled we fallback to the output pixel type.
 */
static inline gp_pixel_type ensure_gray_alpha(const gp_pixel_type_desc *ptd_out)
{
	if (gp_pixel_desc_has_alpha(ptd_out))
		return gp_pixel_desc_pixel_type(ptd_out);

	switch (gp_pixel_desc_channel_size(ptd_out, 'V')) {
#ifdef GP_PIXEL_GA11
	case 1:
		return GP_PIXEL_GA11;
#endif
#ifdef GP_PIXEL_GA22
	case 2:
		return GP_PIXEL_GA22;
#endif
#ifdef GP_PIXEL_GA44
	case 4:
		return GP_PIXEL_GA44;
#endif
#ifdef GP_PIXEL_GA88
	case 8:
		return GP_PIXEL_GA88;
#endif
	default:
		return gp_pixel_desc_pixel_type(ptd_out);
	}
}

static inline gp_pixel dither_to_gray(const gp_pixel_type_desc *ptd_in,
                                      const gp_pixel_type_desc *ptd_out)
{
	switch (ptd_in->flags) {
	case GP_PIXEL_IS_GRAY:
		if (channel_needs_dither(ptd_in, ptd_out, 'V'))
			return gp_pixel_desc_pixel_type(ptd_out);

		return GP_PIXEL_UNKNOWN;

	case GP_PIXEL_IS_GRAY_ALPHA:
		if (channel_needs_dither(ptd_in, ptd_out, 'V'))
			return ensure_gray_alpha(ptd_out);

		return GP_PIXEL_UNKNOWN;

	case GP_PIXEL_IS_RGB:
		return gp_pixel_desc_pixel_type(ptd_out);

	case GP_PIXEL_IS_RGB_ALPHA:
		return ensure_gray_alpha(ptd_out);

	default:
		return gp_pixel_desc_pixel_type(ptd_out);
	}
}

static inline gp_pixel_type dither_to_rgb(const gp_pixel_type_desc *ptd_in,
                                          const gp_pixel_type_desc *ptd_out)
{
	switch (ptd_in->flags) {
	case GP_PIXEL_IS_GRAY:
	case GP_PIXEL_IS_GRAY_ALPHA:
		return GP_PIXEL_UNKNOWN;
	case GP_PIXEL_IS_RGB:
		if (channel_needs_dither(ptd_in, ptd_out, 'R') ||
		    channel_needs_dither(ptd_in, ptd_out, 'G') ||
		    channel_needs_dither(ptd_in, ptd_out, 'B')) {
			return gp_pixel_desc_pixel_type(ptd_out);
		}
		return GP_PIXEL_UNKNOWN;
	// TODO RGB332 and RGB565 format with alpha!
	case GP_PIXEL_IS_RGB_ALPHA:
	default:
		return GP_PIXEL_UNKNOWN;
	}
}

gp_pixel gp_filter_dither_pixel_type(gp_pixel pixel_type_in, gp_pixel pixel_type_out)
{
	const gp_pixel_type_desc *ptd_in = gp_pixel_desc(pixel_type_in);
	const gp_pixel_type_desc *ptd_out = gp_pixel_desc(pixel_type_out);

	switch (ptd_out->flags) {
	case GP_PIXEL_IS_RGB:
	case GP_PIXEL_IS_RGB_ALPHA:
		return dither_to_rgb(ptd_in, ptd_out);

	case GP_PIXEL_IS_GRAY:
	case GP_PIXEL_IS_GRAY_ALPHA:
		return dither_to_gray(ptd_in, ptd_out);

	/* We do not suppport dithering to palette or CMYK */
	default:
		return GP_PIXEL_UNKNOWN;
	}
}
