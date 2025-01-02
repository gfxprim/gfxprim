//SPDX-License-Identifier: LGPL-2.0-or-later

/*

   Copyright (c) 2025 Cyril Hrubis <metan@ucw.cz>

 */

/**
 * @file gp_strconv.h
 * @brief Simple function to convert integers to ascii strings and back.
 */

#ifndef UTILS_GP_STRING_H
#define UTILS_GP_STRING_H

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <core/gp_common.h>

#define GP_XXTOA_ADD(dst, pos, val) \
do { \
	if (dst) \
		dst[pos] = val; \
	pos++; \
} while (0)

#define GP_XXTOA_SWAP(dst, pos) \
do { \
	if (dst) { \
		typeof(pos) i; \
		for (i = 0; i < pos/2; i++) \
			GP_SWAP(dst[i], dst[pos-1-i]); \
	} \
} while (0)

/**
 * @brief Converts an unsigned 32bit integer into an ascii representation.
 *
 * The buffer must be large enough to fit the value. Maximal number of digits
 * for unsigned 32bit integer is 10 not including the terminating nul
 * character.
 *
 * You can run the function with NULL dst to get the exact buffer size for the
 * conversion.
 *
 * @param dst A buffer to write the bytes into. If NULL no bytes are written.
 * @param val An unsigned integer value to be converted to ascii.
 * @param null_terminate If set the string is null terminated.
 * @return A number of bytes written.
 */
static inline size_t gp_u32toa(char *dst, uint32_t val, bool null_terminate)
{
	size_t pos = 0;

	if (!val) {
		GP_XXTOA_ADD(dst, pos, '0');
		goto ret;
	}

	while (val > 0) {
		GP_XXTOA_ADD(dst, pos, '0' + (val % 10));
		val /= 10;
	}

	GP_XXTOA_SWAP(dst, pos);

ret:
	if (null_terminate)
		GP_XXTOA_ADD(dst, pos, 0);

	return pos;
}

/**
 * @brief Converts an unsigned 64bit integer into an ascii representation.
 *
 * The buffer must be large enough to fit the value. Maximal number of digits
 * for unsigned 64bit integer is 20 not including the terminating nul
 * character.
 *
 * You can run the function with NULL dst to get the exact buffer size for the
 * conversion.
 *
 * @param dst A buffer to write the bytes into. If NULL no bytes are written.
 * @param val An unsigned integer value to be converted to ascii.
 * @param null_terminate If set the string is null terminated.
 * @return A number of bytes written.
 */
static inline size_t gp_u64toa(char *dst, uint64_t val, bool null_terminate)
{
	size_t pos = 0;

	if (!val) {
		GP_XXTOA_ADD(dst, pos, '0');
		goto ret;
	}

	while (val > 0) {
		GP_XXTOA_ADD(dst, pos, '0' + (val % 10));
		val /= 10;
	}

	GP_XXTOA_SWAP(dst, pos);

ret:
	if (null_terminate)
		GP_XXTOA_ADD(dst, pos, 0);

	return pos;
}

/**
 * @brief Converts an unsigned integer into an ascii representation.
 *
 * The buffer must be large enough to fit the value. You can run the function
 * with NULL dst to get the exact buffer size for the conversion.
 *
 * @param dst A buffer to write the bytes into. If NULL no bytes are written.
 * @param val An unsigned integer value to be converted to ascii.
 * @param null_terminate If set the string is null terminated.
 * @return A number of bytes written.
 */
static inline size_t gp_utoa(char *dst, unsigned int val, bool null_terminate)
{
#if UINT_MAX == UINT32_MAX
	return gp_u32toa(dst, val, null_terminate);
#elif UINT_MAX == UINT64_MAX
	return gp_u64toa(dst, val, null_terminate);
#else
# error Unexpected unsigned int size
#endif
}

/**
 * @brief Converts an unsigned long into an ascii representation.
 *
 * The buffer must be large enough to fit the value. You can run the function
 * with NULL dst to get the exact buffer size for the conversion.
 *
 * @param dst A buffer to write the bytes into. If NULL no bytes are written.
 * @param val An unsigned integer value to be converted to ascii.
 * @param null_terminate If set the string is null terminated.
 * @return A number of bytes written.
 */
static inline size_t gp_lutoa(char *dst, unsigned long val, bool null_terminate)
{
#if ULONG_MAX == UINT32_MAX
	return gp_u32toa(dst, val, null_terminate);
#elif ULONG_MAX == UINT64_MAX
	return gp_u64toa(dst, val, null_terminate);
#else
# error Unexpected unsigned long size
#endif
}

/**
 * @brief Converts an unsigned long long into an ascii representation.
 *
 * The buffer must be large enough to fit the value. You can run the function
 * with NULL dst to get the exact buffer size for the conversion.
 *
 * @param dst A buffer to write the bytes into. If NULL no bytes are written.
 * @param val An unsigned integer value to be converted to ascii.
 * @param null_terminate If set the string is null terminated.
 * @return A number of bytes written.
 */
static inline size_t gp_llutoa(char *dst, unsigned long long val, bool null_terminate)
{
#if ULLONG_MAX == UINT32_MAX
	return gp_u32toa(dst, val, null_terminate);
#elif ULLONG_MAX == UINT64_MAX
	return gp_u64toa(dst, val, null_terminate);
#else
# error Unexpected unsigned long long size
#endif
}

#define GP_ATOXX_OVERFLOW(cur_val, digit, max) \
	((cur_val > max/10) || (cur_val == max/10 && digit > max%10))

/**
 * @brief Converts an ASCII string into an unsigned 32bit integer.
 *
 * Converts start of the string into an unsigned 32bit integer. The conversion
 * stop either at the end of the string or at first non-numeric character.
 *
 * @param src A source string. The string must be either nul byte terminated or
 *            non-zero src_len has to be passed.
 * @param src_len A number of bytes in src, if zero the string is expected to
 *                be nul byte terminated.
 * @param dst A pointer to an unsigned 32bit integer.
 * @return A number of bytes converted, -1 on overflow.
 */
static inline ssize_t gp_atou32(const char *src, size_t src_len, uint32_t *dst)
{
	size_t pos = 0;
	uint32_t digit;

	*dst = 0;

	while (src[pos] && (!src_len || pos < src_len)) {
		switch (src[pos]) {
		case '0' ... '9':
			digit = src[pos++] - '0';

			if (GP_ATOXX_OVERFLOW(*dst, digit, UINT32_MAX))
				return -1;

			*dst = 10 * (*dst) + digit;
		break;
		default:
			return pos;
		}
	}

	return pos;
}

/**
 * @brief Converts an ASCII string into an unsigned 64bit integer.
 *
 * Converts start of the string into an unsigned 64bit integer. The conversion
 * stop either at the end of the string or at first non-numeric character.
 *
 * @param src A source string. The string must be either nul byte terminated or
 *            non-zero src_len has to be passed.
 * @param src_len A number of bytes in src, if zero the string is expected to
 *                be nul byte terminated.
 * @param dst A pointer to an unsigned 64bit integer.
 * @return A number of bytes converted, -1 on overflow.
 */
static inline ssize_t gp_atou64(const char *src, size_t src_len, uint64_t *dst)
{
	size_t pos = 0;
	uint64_t digit;

	*dst = 0;

	while (src[pos] && (!src_len || pos < src_len)) {
		switch (src[pos]) {
		case '0' ... '9':
			digit = src[pos++] - '0';

			if (GP_ATOXX_OVERFLOW(*dst, digit, UINT64_MAX))
				return -1;

			*dst = 10 * (*dst) + digit;
		break;
		default:
			return pos;
		}
	}

	return pos;
}

/**
 * @brief Converts an ASCII string into an unsigned integer.
 *
 * Converts start of the string into an unsigned integer. The conversion
 * stop either at the end of the string or at first non-numeric character.
 *
 * @param src A source string. The string must be either nul byte terminated or
 *            non-zero src_len has to be passed.
 * @param src_len A number of bytes in src, if zero the string is expected to
 *                be nul byte terminated.
 * @param dst A pointer to an unsigned integer.
 * @return A number of bytes converted, -1 on overflow.
 */
static inline ssize_t gp_atou(const char *src, size_t src_len, unsigned int *dst)
{
#if UINT_MAX == UINT32_MAX
	return gp_atou32(src, src_len, (uint32_t*)dst);
#elif UINT_MAX == UINT64_MAX
	return gp_atou64(src, src_len, (uint64_t*)dst);
#else
# error Unexpected unsigned int size
#endif
}

/**
 * @brief Converts an ASCII string into an unsigned long.
 *
 * Converts start of the string into an unsigned long. The conversion
 * stop either at the end of the string or at first non-numeric character.
 *
 * @param src A source string. The string must be either nul byte terminated or
 *            non-zero src_len has to be passed.
 * @param src_len A number of bytes in src, if zero the string is expected to
 *                be nul byte terminated.
 * @param dst A pointer to an unsigned long.
 * @return A number of bytes converted, -1 on overflow.
 */
static inline ssize_t gp_atolu(const char *src, size_t src_len, unsigned long *dst)
{
#if ULONG_MAX == UINT32_MAX
	return gp_atou32(src, src_len, (uint32_t*)dst);
#elif ULONG_MAX == UINT64_MAX
	return gp_atou64(src, src_len, (uint64_t*)dst);
#else
# error Unexpected unsigned long size
#endif
}

/**
 * @brief Converts an ASCII string into an unsigned long long.
 *
 * Converts start of the string into an unsigned long long. The conversion
 * stop either at the end of the string or at first non-numeric character.
 *
 * @param src A source string. The string must be either nul byte terminated or
 *            non-zero src_len has to be passed.
 * @param src_len A number of bytes in src, if zero the string is expected to
 *                be nul byte terminated.
 * @param dst A pointer to an unsigned long long.
 * @return A number of bytes converted, -1 on overflow.
 */
static inline ssize_t gp_atollu(const char *src, size_t src_len, unsigned long long *dst)
{
#if ULLONG_MAX == UINT32_MAX
	return gp_atou32(src, src_len, (uint32_t*)dst);
#elif ULLONG_MAX == UINT64_MAX
	return gp_atou64(src, src_len, (uint64_t*)dst);
#else
# error Unexpected unsigned long long size
#endif
}

#endif /* UTILS_GP_STRING_H */
