%% extends "base.test.c.t"

%% block body
#include "GP_Tests.h"
#include "GP_WritePixel.h"
#include "GP_TestingCore.h"

GP_SUITE(GP_WritePixel)

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
	unsigned int buf1_len = sizeof(buf1)/sizeof(*buf1);   \
	unsigned int buf2_len = sizeof(buf2)/sizeof(*buf2);   \
	unsigned int i;                                       \
                                                              \
	fail_unless(buf1_len == buf2_len);                    \
                                                              \
	for (i = 0; i < buf1_len; i++)                        \
		if(buf1[i] != buf2[i]) {	              \
			printf("%s\n", id);                   \
			dump_buffer("wrote", buf1, buf1_len); \
			dump_buffer("gen", buf2, buf2_len);   \
			fail("buffers are different");        \
		}                                             \
} while (0)

%% for pixelsize in [8, 16, 24, 32]
%% for offset in range(0, 4)
%% for len    in range(0, 6)
%% for aligment in [0, 1]
GP_TEST(GP_WritePixel{{ "%i_%i_%i_%i"|format(pixelsize, offset, len, aligment) }}, {{ "\"offset=%i, len=%i, aligment=%i,\""|format(offset, len, aligment) }})
{
	char write_buf[{{ 25 * pixelsize//8 }}] = {};
	char gen_buf[{{ 25 * pixelsize//8 }}] = {};

	/*
 	 * Fill the compare buffer
 	 */
%% for i in range(0, len)
%% for j in range(0, pixelsize//8)
	gen_buf[{{aligment + offset * pixelsize//8 + i * pixelsize//8 + j}}] = 0xff;
%% endfor
%% endfor

	GP_WritePixels{{ pixelsize }}bpp(write_buf + {{aligment + offset * pixelsize//8}}, {{ len }}, 0xffffffff>>{{32 - pixelsize}});

	COMPARE_BUFFERS({{"\"p=%i o=%i l=%i a=%i\""|format(pixelsize, offset, len, aligment)}}, write_buf, gen_buf);
}
GP_ENDTEST
%% endfor
%% endfor
%% endfor
%% endfor

%% endblock body
