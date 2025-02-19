// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2011      Tomas Gavenciak <gavento@ucw.cz>
 * Copyright (C) 2011-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_get_set_bits.h
 * @brief Helper macros to get and set bits given offset and length.
 *
 * The GP_GET_BITS() and GP_SET_BITS() works __ONLY__ on aligned data types.
 * Which means that you can only pass value that is suitably aligned for it's
 * type, for example passing an 32 bit integer is OK, passing a char buffer
 * casted to 32 bit integer is not (unless you made sure that the start address
 * is multiple of 4).
 *
 * The align-safe variants first gets the value from a buffer, byte by byte and
 * then uses the GP_GET_BITS() or GP_SET_BITS(). The number in their name tells
 * how much bytes are touched, as we need to touch minimal number of bytes
 * needed.
 */
#ifndef CORE_GP_GET_SET_BITS_H
#define CORE_GP_GET_SET_BITS_H

/**
 * @brief Helper macros to read parts of words
 *
 * @param offset How much to shift the val left.
 * @param len How much bits from val should we keep.
 * @param val A value to be shifted and masked.
 *
 * @return Shifted lenght bits at offset of value.
 */
#define GP_GET_BITS(offset, len, val) \
	(sizeof(val) * 8 <= len ? \
	 (val)>>(offset) : \
	 ((val)>>(offset)) & (((((typeof(val))1)<<(len)) - 1)))

/**
 * @brief Align-safe get bits.
 *
 * Reads four bytes byte by byte, composes then shifts and masks them.
 *
 * @param offset Number of bits to shift to left.
 * @param len A number of bits to return.
 * @param buf A pointer to a starting byte of the buffer.
 */
#define GP_GET_BITS4_ALIGNED(offset, len, buf) ({ \
	uint32_t v;                               \
	v  = ((uint8_t *)buf)[0];                 \
	v |= ((uint8_t *)buf)[1]<<8;              \
	v |= ((uint8_t *)buf)[2]<<16;             \
	v |= ((uint8_t *)buf)[3]<<24;             \
                                                  \
	GP_GET_BITS(offset, len, v);              \
})

/**
 * @brief Align-safe get bits.
 *
 * Reads three bytes byte by byte, composes then shifts and masks them.
 *
 * @param offset Number of bits to shift to left.
 * @param len A number of bits to return.
 * @param buf A pointer to a starting byte of the buffer.
 */
#define GP_GET_BITS3_ALIGNED(offset, len, buf) ({ \
	uint32_t v;                               \
	v  = ((uint8_t *)buf)[0];                 \
	v |= ((uint8_t *)buf)[1]<<8;              \
	v |= ((uint8_t *)buf)[2]<<16;             \
                                                  \
	GP_GET_BITS(offset, len, v);              \
})

/**
 * @brief Align-safe get bits.
 *
 * Reads two bytes byte by byte, composes then shifts and masks them.
 *
 * @param offset Number of bits to shift to left.
 * @param len A number of bits to return.
 * @param buf A pointer to a starting byte of the buffer.
 */
#define GP_GET_BITS2_ALIGNED(offset, len, buf) ({ \
	uint16_t v;                               \
	v  = ((uint8_t *)buf)[0];                 \
	v |= ((uint8_t *)buf)[1]<<8;              \
                                                  \
	GP_GET_BITS(offset, len, v);              \
})

/**
 * @brief Align-safe get bits.
 *
 * Reads byte then shifts and masks it.
 *
 * @param offset Number of bits to shift to left.
 * @param len A number of bits to return.
 * @param buf A pointer to a starting byte of the buffer.
 */
#define GP_GET_BITS1_ALIGNED(offset, len, buf) ({ \
	uint8_t v;                                \
	v = ((uint8_t *)buf)[0];                  \
                                                  \
	GP_GET_BITS(offset, len, v);              \
})

/**
 * @brief Clear len bits at offset in dest.
 *
 * Operates on 8, 16, and 32 bit values, depending on the type of dest, which
 * has to be unsigned.
 *
 * @param offset Number of bits to keep on the left side.
 * @param len Number of bits to clear.
 * @param dest A value to operate on.
 */
#define GP_CLEAR_BITS(offset, len, dest) \
       ((dest) &= ~(((((typeof(dest))1) << (len)) - 1) << (offset)))

/**
 * @brief Set len bits at offset in dest from val.
 *
 * Operates on 8, 16, and 32 bit values, depending on the type of dest, which
 * has to be unsigned.
 *
 * @param offset Number of bits to keep on the left side.
 * @param len Number of bits to clear.
 * @param dest A value to operate on.
 * @param val A value to written to dest.
 */
#define GP_SET_BITS(offset, len, dest, val) do {  \
		GP_CLEAR_BITS(offset, len, dest);  \
		((dest) |= ((val)<<(offset))); \
} while (0)

/**
 * @brief Align-safe set bits.
 *
 * Gets a single byte from dest, combines it with len bytes from value at
 * offset and writes it back.
 *
 * @param offset An offset in the byte.
 * @param len A number of bits to write.
 * @param dest A buffer to write the data to.
 * @param val A value to be written.
 */
#define GP_SET_BITS1_ALIGNED(offset, len, dest, val) do { \
	uint8_t v = ((uint8_t *)dest)[0];                 \
	GP_SET_BITS(offset, len, v, val);                 \
	((uint8_t *)dest)[0] = v;                         \
} while (0)

/**
 * @brief Align-safe set bits.
 *
 * Gets two bytes from dest, combines them with len bytes from value at offset
 * and writes it back.
 *
 * @param offset An offset in the byte.
 * @param len A number of bits to write.
 * @param dest A buffer to write the data to.
 * @param val A value to be written.
 */
#define GP_SET_BITS2_ALIGNED(offset, len, dest, val) do { \
	uint16_t v;                                       \
	v  = ((uint8_t *)dest)[0];                        \
	v |= ((uint8_t *)dest)[1]<<8;                     \
	                                                  \
	GP_SET_BITS(offset, len, v, val);                 \
	                                                  \
	((uint8_t *)dest)[0] = 0xff & v;                  \
	((uint8_t *)dest)[1] = 0xff & (v >> 8);           \
} while (0)

/**
 * @brief Align-safe set bits.
 *
 * Gets three bytes from dest, combines them with len bytes from value at
 * offset and writes it back.
 *
 * @param offset An offset in the byte.
 * @param len A number of bits to write.
 * @param dest A buffer to write the data to.
 * @param val A value to be written.
 */
#define GP_SET_BITS3_ALIGNED(offset, len, dest, val) do { \
	uint32_t v;                                       \
	v  = ((uint8_t *)dest)[0];                        \
	v |= ((uint8_t *)dest)[1]<<8;                     \
	v |= ((uint8_t *)dest)[2]<<16;                    \
	                                                  \
	GP_SET_BITS(offset, len, v, val);                 \
	                                                  \
	((uint8_t *)dest)[0] = 0xff & v;                  \
	((uint8_t *)dest)[1] = 0xff & (v >> 8);           \
	((uint8_t *)dest)[2] = 0xff & (v >> 16);          \
} while (0)

/**
 * @brief Align-safe set bits.
 *
 * Gets four bytes from dest, combines them with len bytes from value at offset
 * and writes it back.
 *
 * @param offset An offset in the byte.
 * @param len A number of bits to write.
 * @param dest A buffer to write the data to.
 * @param val A value to be written.
 */
#define GP_SET_BITS4_ALIGNED(offset, len, dest, val) do { \
	uint32_t v;                                       \
	v  = ((uint8_t *)dest)[0];                        \
	v |= ((uint8_t *)dest)[1]<<8;                     \
	v |= ((uint8_t *)dest)[2]<<16;                    \
	v |= ((uint8_t *)dest)[3]<<24;                    \
	                                                  \
	GP_SET_BITS(offset, len, v, val);                 \
	                                                  \
	((uint8_t *)dest)[0] = 0xff & v;                  \
	((uint8_t *)dest)[1] = 0xff & (v >> 8);           \
	((uint8_t *)dest)[2] = 0xff & (v >> 16);          \
	((uint8_t *)dest)[3] = 0xff & (v >> 24);          \
} while (0)


#endif /* CORE_GP_GET_SET_BITS_H */
