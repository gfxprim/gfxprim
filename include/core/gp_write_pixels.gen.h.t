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

@ def gen_func(suffix, needs_bit_order):
/**
 * @brief Optimized function to draw a row of {{ ps.suffix }} pixels.
 *
 * @param start A pointer to starting byte in the buffer
@     if needs_bit_order:
 * @param off Number of pixels to skip in the first byte
@     end
 * @param cnt A number of pixels to write
 * @param val A pixel value to be written.
 */
@     if needs_bit_order:
void gp_write_pixels_{{ suffix }}(void *start, uint8_t off,
@         if ps.size == 1:
                             uint8_t y_off,
@         end
                             size_t cnt, unsigned int val);

@     else:
void gp_write_pixels_{{ suffix }}(void *start, size_t cnt, unsigned int val);

@ end
@
@ for ps in pixelpacks:
@     if ps.suffix in optimized_writepixels:
@          optimized_writepixels.remove(ps.suffix)
@          gen_func(ps.suffix, ps.needs_bit_order())
@ end
@ for suffix in optimized_writepixels:
@     gen_func(suffix, False)
@ end
