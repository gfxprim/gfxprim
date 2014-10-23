@ include header.t
/*
 * Write Pixels generated header
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * These functions writes cnt pixels using value val starting at start address
 * and additionally (for pixel sizes that are not aligned to the whole bytes)
 * at off offset in the first byte (i.e. byte at the start address).
 */

@ for ps in pixelsizes:
@     if ps.needs_bit_endian():
void GP_WritePixels_{{ ps.suffix }}(void *start, uint8_t off,
                            size_t cnt, unsigned int val);

@     else:
void GP_WritePixels_{{ ps.suffix }}(void *start, size_t cnt, unsigned int val);

