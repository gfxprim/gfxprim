@ include header.t
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_pixel_pack.gen.h
 * @brief Defines how are pixels packed in a buffer.
 */

/**
 * @brief Pixel packings.
 *
 * Pixel packing defines how pixels are packed in a stream of bytes. Note that
 * there may be more than one packing defined for a single pixel size.
 *
 * The bits_per_pixel (pixel size) defines how big is a pixel, it's useful for
 * determining the buffer size, but it does not tell us anything about how
 * pixels are stacked in the buffer. Note that for 4bit depth there are two
 * ways how to interpret which half byte goes first. So there will always be
 * more different packings than pixel sizes.
 */
typedef enum gp_pixel_pack {
@ for pp in pixelpacks:
	/** @brief {{ pp.pack }} */
	{{ pp.pack}} = {{ '0x%02x' % pp.pack_id }},
@ end
} gp_pixel_pack;

@ for pp in pixelpacks:
#define {{ pp.pack }} {{ pp.pack }}
@ end

/**
 * @brief Returns pixel size, e.g. bits per pixel given pixel packing.
 */
uint8_t gp_pixel_bpp_by_pack(enum gp_pixel_pack pack);

/*
 * Branch on pixel packing.
 */
#define GP_FN_RET_PER_PACK(FN_NAME, pack, ...) \
	switch (pack) { \
@ for ps in pixelpacks:
	case {{ ps.pack }}: \
		return FN_NAME##_{{ ps.suffix }}(__VA_ARGS__); \
@ end
	}

/*
 * Branch on pixel packing.
 */
#define GP_FN_PER_PACK(FN_NAME, pack, ...) \
	switch (pack) { \
@ for ps in pixelpacks:
	case {{ ps.pack }}: \
		FN_NAME##_{{ ps.suffix }}(__VA_ARGS__); \
	break; \
@ end
	}

/*
 * Branch on gp_pixel_type argument.
 */
#define GP_FN_PER_PACK_PIXELTYPE(FN_NAME, type, ...) \
	GP_FN_PER_PACK(FN_NAME, gp_pixel_types[type].pack, __VA_ARGS__)

/*
 * Branch on gp_pixel_type argument.
 */
#define GP_FN_RET_PER_PACK_PIXELTYPE(FN_NAME, type, ...) \
	GP_FN_RET_PER_PACK(FN_NAME, gp_pixel_types[type].pack, __VA_ARGS__)

/*
 * Branch on pixmap argument.
 */
#define GP_FN_PER_PACK_PIXMAP(FN_NAME, pixmap, ...) \
	GP_FN_PER_PACK(FN_NAME, gp_pixel_types[pixmap->pixel_type].pack, __VA_ARGS__)

/*
 * Branch on pixmap argument.
 */
#define GP_FN_RET_PER_PACK_PIXMAP(FN_NAME, pixmap, ...) \
	GP_FN_RET_PER_PACK(FN_NAME, gp_pixel_types[pixmap->pixel_type].pack, __VA_ARGS__)

//TODO REMOVE WHEN LAST ALGO IS CONVERTED
#define GP_DEF_DRAW_FN_PER_PACK(fname, MACRO_NAME) \
	GP_DEF_FN_PER_PACK(fname, MACRO_NAME, gp_putpixel_raw_clipped_)

#define GP_DEF_FN_PER_PACK(fname, MACRO_NAME, fdraw) \
@ for ps in pixelpacks:
        GP_DEF_FN_FOR_PACK(fname, MACRO_NAME, fdraw, {{ ps.suffix }}) \
@ end

#define GP_DEF_FN_FOR_PACK(fname, MACRO_NAME, fdraw, bpp) \
        MACRO_NAME(fname##_##bpp, gp_pixmap *, gp_pixel, fdraw##bpp)

#define GP_DEF_FILL_FN_PER_PACK(fname, MACRO_NAME) \
        GP_DEF_FN_PER_PACK(fname, MACRO_NAME, gp_hline_raw_)

