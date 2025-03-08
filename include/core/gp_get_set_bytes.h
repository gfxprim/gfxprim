// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_get_set_bytes.h
 * @brief Helper macros to get and set bytes.
 */
#ifndef CORE_GP_GET_SET_BYTES_H
#define CORE_GP_GET_SET_BYTES_H

#include <core/gp_byte_order.h>

/**
 * @brief Swaps 2 lower bytes in a pixel if pixel is in foreign endianity.
 *
 * @param v A pixel value.
 */
#define GP_SWAP_BYTES2(v) (   \
	(((v) & 0xff00)>>8) | \
	(((v) & 0x00ff)<<8)   \
)

/**
 * @brief Swaps 3 lower bytes in a pixel if pixel is in foreign endianity.
 *
 * @param v A pixel value.
 */
#define GP_SWAP_BYTES3(v) (      \
	 ((v) & 0x00ff00)      | \
	(((v) & 0x0000ff)<<16) | \
	(((v) & 0xff0000)>>16)   \
)

/**
 * @brief Swaps 4 lower bytes in a pixel if pixel is in foreign endianity.
 *
 * @param v A pixel value.
 */
#define GP_SWAP_BYTES4(v) (        \
	(((v) & 0xff000000)>>24) | \
	(((v) & 0x00ff0000)>>8)  | \
	(((v) & 0x0000ff00)<<8)  | \
	(((v) & 0x000000ff)<<24)   \
)

/**
 * @brief Align safe get byte.
 *
 * Reads a byte from a buffer.
 *
 * @param buf A pointer to a starting byte of the buffer.
 */
#define GP_GET_BYTES1(src) (*((uint8_t *)(src)))

/**
 * @brief Align safe set byte.
 *
 * Writes a byte into a buffer buffer.
 *
 * @param buf A pointer to a starting byte of the buffer.
 * @param byte A value to be written.
 */
#define GP_SET_BYTES1(dst, byte) *((uint8_t *)(dst)) = (byte)

/**
 * @brief Little endian align usafe get bytes.
 *
 * Reads two bytes from a buffer in a little endian order.
 *
 * @param buf A pointer to a starting byte of the buffer.
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
# define GP_GET_BYTES2_LE(src) (*((uint16_t *)(src)))
#else
# define GP_GET_BYTES2_LE(src) GP_SWAP_BYTES2(*((uint16_t *)(src)))
#endif

/**
 * @brief Little endian align usafe set bytes.
 *
 * Writes two bytes from to a buffer in a little endian order.
 *
 * @param buf A pointer to a starting byte of the buffer.
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
# define GP_SET_BYTES2_LE(dst, bytes) *((uint16_t *)(dst)) = (bytes)
#else
# define GP_SET_BYTES2_LE(dst, bytes) *((uint16_t *)(dst)) = GP_SWAP_BYTES2(bytes)
#endif

/**
 * @brief Big endian align usafe get bytes.
 *
 * Reads two bytes from a buffer in a big endian order.
 *
 * @param buf A pointer to a starting byte of the buffer.
 */
#if __BYTE_ORDER == __BIG_ENDIAN
# define GP_GET_BYTES2_BE(src) (*((uint16_t *)(src)))
#else
# define GP_GET_BYTES2_BE(src) GP_SWAP_BYTES2(*((uint16_t *)(src)))
#endif

/**
 * @brief Big endian align usafe set bytes.
 *
 * Writes two bytes from to a buffer in a big endian order.
 *
 * @param buf A pointer to a starting byte of the buffer.
 */
#if __BYTE_ORDER == __BIG_ENDIAN
# define GP_SET_BYTES2_BE(dst, bytes) *((uint16_t *)(dst)) = (bytes)
#else
# define GP_SET_BYTES2_BE(dst, bytes) *((uint16_t *)(dst)) = GP_SWAP_BYTES2(bytes)
#endif

/**
 * @brief Big endian align usafe get bytes.
 *
 * Reads two bytes from a buffer in a big endian order.
 *
 * @param buf A pointer to a starting byte of the buffer.
 */
#if __BYTE_ORDER == __BIG_ENDIAN
# define GP_GET_BYTES4_BE(src) *((uint32_t *)src)
#else
# define GP_GET_BYTES4_BE(src) GP_SWAP_BYTES4(*((uint16_t *)(src)))
#endif

/**
 * @brief Big endian align usafe set bytes.
 *
 * Writes four bytes from to a buffer in a big endian order.
 *
 * @param buf A pointer to a starting byte of the buffer.
 */
#if __BYTE_ORDER == __BIG_ENDIAN
# define GP_SET_BYTES4_BE(dst, bytes) *((uint32_t *)(dst)) = (bytes)
#else
# define GP_SET_BYTES4_BE(dst, bytes) *((uint32_t *)(dst)) = GP_SWAP_BYTES4(bytes)
#endif

/**
 * @brief Little endian align usafe get bytes.
 *
 * Reads two bytes from a buffer in a little endian order.
 *
 * @param buf A pointer to a starting byte of the buffer.
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
# define GP_GET_BYTES4_LE(src) *((uint32_t *)src)
#else
# define GP_GET_BYTES4_LE(src) GP_SWAP_BYTES4(*((uint16_t *)(src)))
#endif

/**
 * @brief Little endian align usafe set bytes.
 *
 * Writes four bytes from to a buffer in a little endian order.
 *
 * @param buf A pointer to a starting byte of the buffer.
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
# define GP_SET_BYTES4_LE(dst, bytes) *((uint32_t *)(dst)) = (bytes)
#else
# define GP_SET_BYTES4_LE(dst, bytes) *((uint32_t *)(dst)) = GP_SWAP_BYTES4(bytes)
#endif

#endif /* CORE_GP_GET_SET_BYTES_H */
