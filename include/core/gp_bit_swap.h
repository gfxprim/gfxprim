// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2012 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @file gp_bit_swap.h
  * @brief Inline functions for swapping bits inside of the byte.
  */

#ifndef CORE_GP_BIT_SWAP_H
#define CORE_GP_BIT_SWAP_H

/**
 * @brief Reverses 1 bit blocks in the byte.
 *
 * Example:
 *
 * IN  10101110
 * OUT 01110101
 *
 * @param byte An input byte.
 * @return A reversed byte.
 */
static inline uint8_t GP_BIT_SWAP_B1(uint8_t byte)
{
	return ((byte * 0x0802LU & 0x22110LU) |
                (byte * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

/**
 * @brief Reverses 1 bit blocks in a row.
 *
 * @param row A pointer to a start of a row.
 * @param len A number of bytes to reverse.
 */
static inline void gp_bit_swap_row_b1(uint8_t *row, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		row[i] = GP_BIT_SWAP_B1(row[i]);
}

/**
 * @brief Reverses 2 bit blocks in the byte.
 *
 * Example:
 *
 * IN  10 11 01 00
 * OUT 00 01 11 10
 *
 * @param byte An input byte.
 * @return A reversed byte.
 */
static inline uint8_t GP_BIT_SWAP_B2(uint8_t byte)
{
	return ((byte & 0xC0) >> 6) | ((byte & 0x30) >> 2) |
	       ((byte & 0x0C) << 2) | ((byte & 0x03) << 6);
}

/**
 * @brief Reverses 2 bit blocks in a row.
 *
 * @param row A pointer to a start of a row.
 * @param len A number of bytes to reverse.
 */
static inline void gp_bit_swap_row_b2(uint8_t *row, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		row[i] = GP_BIT_SWAP_B2(row[i]);
}

/**
 * @brief Reverses 4 bit blocks in the byte.
 *
 * Example:
 *
 * IN  1011 0100
 * OUT 0100 1011
 *
 * @param byte An input byte.
 * @return A reversed byte.
 */
static inline uint8_t GP_BIT_SWAP_B4(uint8_t byte)
{
	return ((byte & 0xf0) >> 4) | ((byte & 0x0f) << 4);
}

/**
 * @brief Reverses 4 bit blocks in a row.
 *
 * @param row A pointer to a start of a row.
 * @param len A number of bytes to reverse.
 */
static inline void gp_bit_swap_row_b4(uint8_t *row, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++)
		row[i] = GP_BIT_SWAP_B4(row[i]);
}

#endif /* CORE_GP_BIT_SWAP_H */
