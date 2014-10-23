/*
 * WritePixel tests.
 *
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <core/GP_WritePixel.h>

#include "tst_test.h"

static void dump_buffer(const char *name, char *buf, unsigned int buf_len)
{
	unsigned int i;

	printf("%s:\n{", name);

	for (i = 0; i < buf_len; i++) {
		printf("%i", !!buf[i]);

		if (i != buf_len - 1)
			printf(", ");

		if (i % 26 == 25)
			printf("\n ");
	}

	printf("}\n");
}

/*
 * Compares two statically defined buffers
 */
#define COMPARE_BUFFERS(id, buf1, buf2) do {                  \
	unsigned int buf_len = sizeof(buf2)/sizeof(*buf2);    \
	unsigned int i;                                       \
                                                              \
	                                                      \
	for (i = 0; i < buf_len; i++)                         \
		if(buf1[i] != buf2[i]) {	              \
			printf("%s\n", id);                   \
			dump_buffer("wrote", buf1, buf_len);  \
			dump_buffer("gen", buf2, buf_len);    \
			tst_msg("Buffers are different");     \
			return TST_FAILED;                    \
		}                                             \
	                                                      \
	return TST_SUCCESS;                                   \
} while (0)

@ for pixelsize in [8, 16, 24, 32]:
@     for offset in range(0, 4):
@         for len in range(0, 6):
@             for aligment in [0, 4]:
@                 if (pixelsize != 16 and pixelsize != 32) or aligment == 0:
static int WritePixel{{ "_%i_%i_%i_%i" % (pixelsize, offset, len, aligment) }}(void)
{
	char write_buf[{{ 25 * pixelsize//8 }}] = {};
	char gen_buf[{{ 25 * pixelsize//8 }}] = {};

	/*
	 * Fill the compare buffer
	 */
@                     for i in range(0, len):
@                         for j in range(0, pixelsize//8):
	gen_buf[{{aligment + offset * pixelsize//8 + i * pixelsize//8 + j}}] = 0xff;
@                     end

	GP_WritePixels_{{ pixelsize }}BPP(write_buf + {{aligment + offset * pixelsize//8}}, {{ len }}, 0xffffffff>>{{32 - pixelsize}});

	COMPARE_BUFFERS({{'"p=%i o=%i l=%i a=%i"' % (pixelsize, offset, len, aligment)}}, write_buf, gen_buf);
}

static int WritePixel{{ "_%i_%i_%i_%i_alloc" % (pixelsize, offset, len, aligment) }}(void)
{
	char gen_buf[{{ 25 * pixelsize//8 }}] = {};
	char *write_buf = malloc({{ 25 * pixelsize//8 }});

	/*
	 * Fill the compare buffer
	 */
@                     for i in range(0, len):
@                         for j in range(0, pixelsize//8):
	gen_buf[{{aligment + offset * pixelsize//8 + i * pixelsize//8 + j}}] = 0xff;
@                     end

	if (gen_buf == NULL) {
		tst_msg("Malloc failed :(");
		return TST_UNTESTED;
	}

	memset(write_buf, 0, {{ 25 * pixelsize//8 }});

	GP_WritePixels_{{ pixelsize }}BPP(write_buf + {{aligment + offset * pixelsize//8}}, {{ len }}, 0xffffffff>>{{32 - pixelsize}});

	COMPARE_BUFFERS({{'"p=%i o=%i l=%i a=%i"' % (pixelsize, offset, len, aligment)}}, write_buf, gen_buf);
}
@ end

const struct tst_suite tst_suite = {
	.suite_name = "WritePixel Testsuite",
	.tests = {
@ for pixelsize in [8, 16, 24, 32]:
@     for offset in range(0, 4):
@         for len in range(0, 6):
@             for aligment in [0, 4]:
@                 if (pixelsize != 16 and pixelsize != 32) or aligment == 0:
		{.name = "WritePixel {{ pixelsize }} {{ offset }} {{ len }} {{ aligment }} stack",
		 .tst_fn = WritePixel{{ "_%i_%i_%i_%i" % (pixelsize, offset, len, aligment) }}},
		{.name = "WritePixel {{ pixelsize }} {{ offset }} {{ len }} {{ aligment }} alloc",
		 .tst_fn = WritePixel{{ "_%i_%i_%i_%i_alloc" % (pixelsize, offset, len, aligment) }}},
@ end
		{.name = NULL}
	}
};
