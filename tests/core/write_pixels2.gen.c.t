/*
 * write_pixel tests.
 *
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <core/gp_write_pixels.gen.h>

#include "tst_test.h"

static void dump_buffers(uint8_t *exp, uint8_t *res, unsigned int len)
{
	unsigned int i;

	printf("exp: ");

	for (i = 0; i < len; i++)
		printf("%02x ", exp[i]);

	printf("\n");

	printf("res: ");

	for (i = 0; i < len; i++)
		printf("%02x ", res[i]);

	printf("\n");
}

static int compare_buffers(uint8_t *exp, uint8_t *res, unsigned int len)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		if (exp[i] != res[i]) {
			dump_buffers(exp, res, len);
			return TST_FAILED;
		}
	}

	return TST_PASSED;
}

@ max_buf_len = 64
@ max_len = 32
@ max_off = 8
@
@ def gen_bits(off, len):
@     bits = [];
@     for i in range(0, off):
@         bits.append(0)
@     for i in range(0, len):
@         bits.append(1)
@     for i in range(0, max_buf_len - off - len):
@         bits.append(0)
@     return bits
@
@ def gen_bytes(off, len, endian):
@     bits = gen_bits(off, len)
@     bytes = []
@     for j in range(0, int(max_buf_len/8)):
@         byte = 0;
@         for i in range(0, 8):
@             if endian == 'DB':
@                 byte = byte | (bits[i + j * 8] << i)
@             else:
@                 byte = byte | (bits[i + j * 8] << (7-i))
@         bytes.append(byte)
@     end
@     return bytes
@
@ for endian in ['DB', 'UB']:
@     for off in range(0, max_off):
@         for len in range(0, max_len):
static int write_pixels_1BPP_{{endian}}_{{off}}_{{len}}(void)
{
@             bytes = gen_bytes(off, len, endian)
	uint8_t exp[] = {
		0x00,
		0x00,
@             for i in bytes:
		{{'0x%02x' % i}},
@             end
	};

	uint8_t res[{{int(max_buf_len/8)+2}}] = {};

	gp_write_pixels_1BPP_{{endian}}(res+2, {{off}}, {{len}}, 1);

	return compare_buffers(exp, res, {{int(max_buf_len/8)+2}});
}
@ end

const struct tst_suite tst_suite = {
	.suite_name = "write_pixels 1BPP tests",
	.tests = {
@ for endian in ['DB', 'UB']:
@     for bpp in [1]:
@         for off in range(0, max_off):
@             for len in range(0, max_len):
		{.name = "write_pixels {{bpp}}BPP_{{endian}} {{off}} {{len}}",
		 .tst_fn = write_pixels{{ "_%iBPP_%s_%i_%i" % (bpp, endian, off, len) }}},
@ end
		{}
	}
};
