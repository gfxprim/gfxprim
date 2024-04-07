@ include header.t
@ include write_pixels.t
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */
/**
 * @file gp_write_pixels.gen.h
 * @brief Optimized functions to write a row of pixels.
 *
 * These functions writes cnt pixels using value val starting at start address
 * and additionally (for pixel sizes that are not aligned to the whole bytes)
 * at off offset in the first byte (i.e. byte at the start address).
 */
#include <stddef.h>
#include <stdint.h>

@ for ps in pixelpacks:
@     if ps.suffix in optimized_writepixels:
/**
 * @brief Optimized function to draw a row of {{ ps.suffix }} pixels.
 *
 * @param start A pointer to starting byte in the buffer
@         if ps.needs_bit_order():
 * @param off Number of pixels to skip in the first byte
@         end
 * @param cnt A number of pixels to write
 * @param val A pixel value to be written.
 */
@         if ps.needs_bit_order():
void gp_write_pixels_{{ ps.suffix }}(void *start, uint8_t off,
                             size_t cnt, unsigned int val);

@         else:
void gp_write_pixels_{{ ps.suffix }}(void *start, size_t cnt, unsigned int val);

@ end
