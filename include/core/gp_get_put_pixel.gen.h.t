@ include header.t
/*
 * Access pixel bytes, get and put pixel
 * Do not include directly, use gp_pixel.h
 *
 * Copyright (C) 2011-2025 Cyril Hrubis <metan@ucw.cz>
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 */

/**
 * @file gp_get_put_pixel.gen.h
 * @brief Generated macros and function to access pixels.
 *
 * Generated macros to access pixel data
 * =====================================
 *
 * These function and macros are generated based on the pixel defintions in
 * gfxprim_config.py. The get and put pixel functions are build on the top of
 * macros to manipulate bytes and bits.
 *
 * The GP_PIXEL_ADDR macros return a pointer to the first byte of the pixel in
 * the gp_pixmap buffer. If the pixel is aligned to whole bytes, we just need
 * to fetch/write a few bytes. Otherwise we need to compute the shift, fetch a
 * few bytes and then mask and bit shift it.
 *
 * Note about byte aligment
 * ========================
 *
 * Unaligned access happens when instruction that works with multiple byte
 * value gets an address that is not divideable by the size of the value. Eg.
 * if 32 bit integer instruction gets an address that is not a multiple of 4.
 * On intel cpus this type of access works and is supported however the C
 * standard defines this as undefined behavior. This fails to work older ARM
 * and most of the non intel cpus. So some more trickery must be done in order
 * to write unaligned multibyte values. First of all we must compute offset and
 * number of bytes to be accessed (which is cruicial for speed as we are going
 * to read the pixel value byte by byte).
 *
 * The offsets (starting with the first one e.g. pixel_size mod 8) forms subgroup
 * in the mod 8 cyclic group. The maximal count of bits, from the start of the
 * byte, then will be max from this subgroup + pixel_size. If this number is
 * less or equal to 8 * N, we could write such pixel by writing N bytes.
 *
 * For example the offsets of 16 BPP forms subgroup only with {0} so we only
 * need 2 bytes to write it. As a matter of fact the 16 and 32 BPP are special
 * cases that are always aligned together with the 8 BPP (which is aligned
 * trivially). These three are coded as special cases which yields to faster
 * operations in case of 16 and 32 BPP. The 24 BPP is not aligned as there are
 * no instruction to operate 3 byte long numbers.
 *
 * For second example take offsets of 20 BPP that forms subgroup {4, 0}
 * so the max + pixel_size = 24 and indeed we fit into 3 bytes.
 *
 * If pixel_size is coprime to 8, the offsets generates whole group and so the
 * max + pixel_size = 7 + pixel_size. The 17 BPP fits into 24 bits and so 3
 * bytes are needed. The 19 BPP fits into 26 bits and because of that 4 bytes
 * are needed.
 *
 * Once we figure maximal number of bytes and the offset all that is to be done
 * is to fetch first and last byte to combine it together with given pixel value
 * and write the result back to the bitmap.
 */

#include <core/gp_get_set_bits.h>
#include <core/gp_get_set_bytes.h>
#include <core/gp_pixmap.h>

@ for ps in pixelpacks:

/**
 * @brief A macro to get address of a starting byte for a pixel in a
 *        {{ ps.suffix }} pixmap.
 *
 * @param pixmap A pixmap.
 * @param x An x coordinate within the pixmap.
 * @param y An y coordinate within the pixmap.
 */
#define GP_PIXEL_ADDR_{{ ps.suffix }}(pixmap, x, y) \
	((gp_pixel*)(((void*)((pixmap)->pixels)) + (pixmap)->bytes_per_row * (y) + \
@     if ps.needs_bit_order():
	({{ ps.size }} * ((pixmap)->offset + x)) / 8))
@     else:
	({{ ps.size }} * (x)) / 8))
@     end

/**
 * @brief A macro to get bit-shift of pixel in {{ ps.suffix }} pixmap.
 *
 * @param pixmap A pixmap.
 * @param x An x coordinate in the pixmap.
 */
#define GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(pixmap, x) \
@     if not ps.needs_bit_order():
	(0)
@     else:
@         if ps.bit_order == 'DB':
@             if ps.size < 8:
	(((x + (pixmap)->offset) % {{ 8 // ps.size }}) * {{ ps.size }})
@             else:
	(({{ ps.size }} * (x + (pixmap)->offset)) % 8)
@         else:
@              if ps.size < 8:
	({{ 8 - ps.size }} - ((x + (pixmap)->offset) % {{ 8 // ps.size }}) * {{ ps.size }})
@              else:
	{{ error('Insanity check: big bit-endian with >8 bpp. Are you sure?') }}
@     end

/**
 * @brief A macro to get pixel offset in pixels.
 */
#define GP_PIXEL_ADDR_OFFSET_{{ ps.suffix }}(pixmap, x) \
@     if not ps.needs_bit_order():
	(0)
@     elif ps.size < 8:
	(((pixmap)->offset + x) % {{ int(8 / ps.size) }})
@     else:
@ #TODO
	(0)
@     end

/**
 * @brief gp_getpixel for {{ ps.suffix }}
 *
 * @warning The x and y must be inside of the pixmap!
 *
 * @param c A pixmap.
 * @param x A x coordinate.
 * @param y A y coordinate.
 *
 * @return A pixel value at x, y.
 */
static inline gp_pixel gp_getpixel_raw_{{ ps.suffix }}(const gp_pixmap *c, gp_coord x, gp_coord y)
{
@     if ps.size == 32:
	/*
	 * 32 BPP is expected to have aligned pixels
	 */
	return GP_GET_BYTES4_{{ ps.endian }}(GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y));
@     elif ps.size == 16:
	/*
	 * 16 BPP is expected to have aligned pixels
	 */
	return GP_GET_BYTES2_{{ ps.endian }}(GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y));
@     elif ps.size == 8:
	/*
	 * 8 BPP is byte aligned
	 */
	return GP_GET_BYTES1(GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y));
@     elif ps.size == 1 or ps.size == 2 or ps.size == 4:
	/*
	 * Whole pixel is stored in one byte
	 *
	 * The full list = {1, 2, 4}
	 */
	return GP_GET_BITS1(GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(c, x), {{ ps.size }},
	                    GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y));
@     elif ps.size <= 10 or ps.size == 12 or ps.size == 16:
	/*
	 * The pixel is stored in one or two bytes
	 *
	 * The max from subgroup (of mod 8 factor group) generated by
	 * pixel_size mod 8 + pixel_size <= 16
	 *
	 * The full list = {3, 5, 6, 7, 9, 10, 12, 16}
	 *
	 * Hint: If the pixel size is coprime to 8 the group is generated by
	 *       pixel_size mod 8 and maximal size thus is pixel_size + 7
	 */
	return GP_GET_BITS2_{{ ps.endian }}(GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(c, x), {{ ps.size }},
	                    GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y));
@     elif ps.size <= 18 or ps.size == 20 or ps.size == 24:
	/*
	 * The pixel is stored in two or three bytes
	 *
	 * The max from subgroup (of mod 8 factor group) generated by
	 * pixel_size mod 8 + pixel_size <= 24
	 *
	 * The full list = {11, 13, 14, 15, 17, 18, 20, 24}
	 *
	 * Hint: If the pixel size is coprime to 8 the group is generated by
	 *       pixel_size mod 8 and maximal size thus is pixel_size + 7
	 */
	return GP_GET_BITS3_{{ ps.endian }}(GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(c, x), {{ ps.size }},
	                    GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y));
@     elif ps.size <= 23 or ps.size == 25 or ps.size == 26 or ps.size == 28 or ps.size == 32:
	/*
	 * The pixel is stored in three or four bytes
	 *
	 * The max from subgroup (of mod 8 factor group) generated by
	 * pixel_size mod 8 + pixel_size <= 32
	 *
	 * The full list = {19, 21, 22, 23, 25, 26, 28, 32}
	 *
	 * Hint: If the pixel size is coprime to 8 the group is generated by
	 *       pixel_size mod 8 and maximal size thus is pixel_size + 7
	 */
	return GP_GET_BITS4_{{ ps.endian }}(GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(c, x), {{ ps.size }},
	                    GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y));
@     else:
	#error not implemented
@     end
}

/**
 * @brief gp_putpixel for {{ ps.suffix }}
 *
 * @warning The x and y must be inside of the pixmap!
 *
 * @param c A pixmap.
 * @param x A x coordinate.
 * @param y A y coordinate.
 * @param p A pixel value to be written into the buffer.
 */
static inline void gp_putpixel_raw_{{ ps.suffix }}(gp_pixmap *c, gp_coord x, gp_coord y, gp_pixel p)
{
@     if ps.size == 32:
	/*
	 * 32 BPP is expected to have aligned pixels
	 */
	GP_SET_BYTES4_{{ ps.endian }}(GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y), p);
@     elif ps.size == 16:
	/*
	 * 16 BPP is expected to have aligned pixels
	 */
	GP_SET_BYTES2_{{ ps.endian }}(GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y), p);
@     elif ps.size == 8:
	/*
	 * 8 BPP is byte aligned
	 */
	GP_SET_BYTES1(GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y), p);
@     elif ps.size == 1 or ps.size == 2 or ps.size == 4:
	/*
	 * Whole pixel is stored one byte
	 *
	 * The full list = {1, 2, 4}
	 */
	GP_SET_BITS1(GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(c, x), {{ ps.size }},
	             GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y), p);
@     elif ps.size <= 10 or ps.size == 12 or ps.size == 16:
	/*
	 * The pixel is stored in one or two bytes
	 *
	 * The max from subgroup (of mod 8 factor group) generated by
	 * pixel_size mod 8 + pixel_size <= 16
	 *
	 * The full list = {3, 5, 6, 7, 9, 10, 12, 16}
	 *
	 * Hint: If the pixel size is coprime to 8 the group is generated by
	 *       pixel_size mod 8 and maximal size thus is pixel_size + 7
	 */
	GP_SET_BITS2_{{ ps.endian }}(GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(c, x), {{ ps.size }},
	             GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y), p);
@     elif ps.size <= 18 or ps.size == 20 or ps.size == 24:
	/*
	 * The pixel is stored in two or three bytes
	 *
	 * The max from subgroup (of mod 8 factor group) generated by
	 * pixel_size mod 8 + pixel_size <= 24
	 *
	 * The full list = {11, 13, 14, 15, 17, 18, 20, 24}
	 *
	 * Hint: If the pixel size is coprime to 8 the group is generated by
	 *       pixel_size mod 8 and maximal size thus is pixel_size + 7
	 */
	GP_SET_BITS3_{{ ps.endian }}(GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(c, x), {{ ps.size }},
	             GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y), p);
@     elif ps.size <= 23 or ps.size == 25 or ps.size == 26 or ps.size == 28 or ps.size == 32:
	/*
	 * The pixel is stored in three or four bytes
	 *
	 * The max from subgroup (of mod 8 factor group) generated by
	 * pixel_size mod 8 + pixel_size <= 32
	 *
	 * The full list = {19, 21, 22, 23, 25, 26, 28, 32}
	 *
	 * Hint: If the pixel size is coprime to 8 the group is generated by
	 *       pixel_size mod 8 and maximal size thus is pixel_size + 7
	 */
	GP_SET_BITS4__{{ ps.endian }}(GP_PIXEL_ADDR_SHIFT_{{ ps.suffix }}(c, x), {{ ps.size }},
	             GP_PIXEL_ADDR_{{ ps.suffix }}(c, x, y), p);
@     else:
	#error not implemented
@     end
}

static inline void gp_putpixel_raw_clipped_{{ ps.suffix }}(gp_pixmap *c, gp_coord x, gp_coord y, gp_pixel p)
{
	if (GP_PIXEL_IS_CLIPPED(c, x, y))
		return;

	gp_putpixel_raw_{{ ps.suffix }}(c, x, y, p);
}
