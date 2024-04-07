// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/**
 * @file gp_gamma_correction.h
 * @brief Gamma and sRGB corrections.
 * @defgroup gamma_correction Gamma correction
 *
 * What is gamma?
 * ==============
 *
 * First of all gamma is a function, or better there is a gamma function and
 * it's inverse function. Both gamma function and it's inverse are defined on
 * interval [0,1] and are defined as out = in^(gamma) and it's inverse as
 * out = in^(1/gamma).
 *
 * The purpose of this function is to compensate nonlinearity of human eye
 * perception. The human eye is more sensitive to dark tones than the light ones
 * so without gamma correction storage and manipulation with image data would
 * either be less efficient in space (in case you decided to use more bits and
 * encode the image linearly) or quantization in darker tones would be more
 * visible resulting in "pixelated" images (aliasing).
 *
 * So there is a gamma, the Internet seems to suggest that usual values for
 * gamma are 2.5 for old CRT monitors and about 2.2 for LCD ones, ideally you
 * should have color profile for your device (you need special hardware to
 * measure it). So if you are trying to draw linear gradient on the screen
 * you need to generate sequence of numbers accordingly to gamma function
 * (the 50% intensity is around 186 for gamma = 2.2 and 8bit grayscale pixel).
 *
 * Moreover image formats tend to save data in nonlinear fashion (some formats
 * include gamma value used to for the image) so before you apply filter that
 * manipulates with pixel values, you need to convert it to linear space (adding
 * some more bits to compensate for rounding errors).
 *
 * Also it's important to take gamma, into an account, when drawing anti-aliased
 * shapes, you can't get right results otherwise.
 *
 * Implementation
 * ==============
 *
 * This code implements management functions for easy, per pixmap, per
 * channel, gamma tables.
 *
 * The tables for particular gamma are reference counted. There is only one
 * table for particular gamma value and bit depth in memory at a time.
 *
 * The pointers to gamma tables are storied in gp_gamma structure and are
 * organized in the same order as channels. The lin tables linearize pixel
 * values and use at least two more bits than the input size. The enc tables
 * encode the linear values back into the original non-linear space.
 *
 * E.g. if we have RGB888 pixel and gamma 2.2 there are two lookup tables in
 * the memory, one for gamma=2.2 8bit -> 10bit that's an array of 256 x u16
 * values and its inverse gamma=0.4545... 10bit -> 8bit that's an array 1024 x
 * u8 values. The gp_gamma lin[] first three pointers point to the first table
 * and the first three pointers in enc[] points to the second table.
 *
 * To get a linear value for a RGB888 pixel channel:
 * @code
 * uint32_t chan_val_lin = gamma->lin[chan_number].u16[chan_val];
 * @endcode
 *
 * And to get the value back:
 * @code
 * uint8_t chan_val = gamma->enc[chan_number].u8[chan_val_lin];
 * @endcode
 *
 * When doing more than one conversion it's better to save pointers to
 * individual table:
 *
 * @code
 * uint16_t *R_2_LIN = gamma->lin[0].u16;
 * ...
 * uint8_t *R_2_GAMMA = gamma->enc[0].u8;
 * ...
 * @endcode
 */
#ifndef CORE_GP_GAMMA_CORRECTION_H
#define CORE_GP_GAMMA_CORRECTION_H

#include <stdint.h>
#include <core/gp_types.h>
#include <core/gp_pixel.gen.h>
#include <core/gp_gamma_correction.gen.h>

/**
 * @brief A list of supported correction types
 * @ingroup gamma_correction
 */
typedef enum gp_correction_type {
	/** @brief Classical gamma correction */
	GP_CORRECTION_GAMMA,
	/** @brief Standard RGB - gamma = 2.2 linearized near zero */
	GP_CORRECTION_SRGB,
} gp_correction_type;

/**
 * @brief A lookup gamma table.
 */
typedef struct gp_gamma_table {
	/** @brief Correction type */
	gp_correction_type type;
	/** @brief Gamma for GP_CORRECTION_GAMMA */
	float gamma;

	/** @brief Number of input bits */
	uint8_t in_bits;
	/** @brief Number of output bits */
	uint8_t out_bits;

	/** @brief Reference counter */
	unsigned int ref_count;
	struct gp_gamma_table *next;

	/** @brief Lookup table */
	union {
		uint8_t u8[0];
		uint16_t u16[0];
	};
} gp_gamma_table;

/**
 * @brief A correction tables for all pixel channels.
 *
 * Contains pointers to lookup tables for each pixel channel for linearization
 * and inverse transformation. Linearization produces a values with a few more
 * bits, e.g. for 8bit value you end up with at least 10bits to correctly
 * encode the linearized values.
 */
typedef struct gp_gamma {
	/** @brief Pixel type the table could be used for */
	gp_pixel_type pixel_type;

	/** @brief Reference counter */
	unsigned int ref_count;

	/** @brief Tables to linearize channel values */
	gp_gamma_table *lin[GP_PIXEL_CHANS_MAX];
	/** @brief Tables to encode the values back */
	gp_gamma_table *enc[GP_PIXEL_CHANS_MAX];
} gp_gamma;

/**
 * @brief Acquires a gamma tables for a given pixel type, correction type and
 *        a gamma value.
 * @ingroup gamma_correction
 *
 * For a GP_CORRECTION_SRGB the gamma value is ignored.
 *
 * @param pixel_type A pixel type to build the tables for
 * @param corr_type A correction type.
 * @param gamma A gamma value, rounded to three decimal places internally.
 *
 * @return A gamma correction tables.
 */
gp_gamma *gp_gamma_acquire(gp_pixel_type pixel_type,
                           gp_correction_type corr_type, float gamma);

/**
 * @brief Increases reference counters.
 *
 * Increases reference counters for gamma tables. Each time a pixmap is
 * duplicated or new pixmap is created as a result of filters the newly created
 * pixmap needs to inherit the gamma correction tables.
 *
 * @param gamma A gamma table.
 * @return A pointer to the gamma table passed as argument.
 */
gp_gamma *gp_gamma_incref(gp_gamma *gamma);

/**
 * @brief Decreases reference counters.
 *
 * Decreases reference counters and frees the table if counters drop to zero.
 *
 * @param self A gamma table.
 */
void gp_gamma_decref(gp_gamma *self);

/**
 * @brief Returns correction name.
 *
 * @param type A correction type.
 * @return A correction name.
 */
const char *gp_correction_type_name(gp_correction_type type);

/**
 * @brief Prints info about gamma table into the stdout.
 *
 * @param self A gamma table.
 */
void gp_gamma_print(const gp_gamma *self);

#endif /* CORE_GP_GAMMA_CORRECTION_H */
