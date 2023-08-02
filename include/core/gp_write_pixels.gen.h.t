@ include header.t
@ include write_pixels.t
/*
 * Write Pixels generated header
 *
 * Copyright (C) 2009-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <stddef.h>
#include <stdint.h>

/*
 * These functions writes cnt pixels using value val starting at start address
 * and additionally (for pixel sizes that are not aligned to the whole bytes)
 * at off offset in the first byte (i.e. byte at the start address).
 *
 * @start A starting byte in the buffer
 * @off Number of pixels to skip in the first byte
 * @cnt A number of pixels to write
 * @val A value to be written.
 */
@ for ps in pixelpacks:
@   if ps.suffix in optimized_writepixels:
@     if ps.needs_bit_order():
void gp_write_pixels_{{ ps.suffix }}(void *start, uint8_t off,
                             size_t cnt, unsigned int val);

@     else:
void gp_write_pixels_{{ ps.suffix }}(void *start, size_t cnt, unsigned int val);

@ end
