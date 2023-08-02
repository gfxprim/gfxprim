@ include source.t
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

static uint8_t gp_pixel_bpp_by_pack(enum gp_pixel_pack pack)
{
	switch (pack) {
@ for ps in pixelsizes:
	case {{ ps.pack }}:
		return {{ ps.size }};
@ end
	}

	GP_WARN("Invalid packing %u\n", pack);
	return 0;
}
