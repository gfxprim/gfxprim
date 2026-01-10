// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2025-2026 Cyril Hrubis <metan@ucw.cz>
 */

#include <backends/gp_backend.h>
#include <backends/gp_display_st75256.h>
#include <core/gp_pixmap.h>
#include <core/gp_debug.h>

#include "gp_display_eink.h"
#include "gp_display_conn.h"

/*
 * ST75256 LCD driver constants
 *
 * The commands are split into extensions, proper extension has to be selected.
 */
enum st7565_cmds {
	/* Sets Extension1 commands. */
	ST75256_EXT1        = 0x30,
	/* Sets Extension2 commands. */
	ST75256_EXT2        = 0x31,
	/* Sets Extension3 commands. */
	ST75256_EXT3        = 0x38,
	/* Sets Extension4 commands. */
	ST75256_EXT4        = 0x39,

	/* Extension1 commands */
	/**
	 * Power controll, followed by a byte.
	 *
	 * bit or of:
	 * 0x01 VR Internal regulator on/off
	 * 0x02 VF Internal follower on/off
	 * 0x08 VB Internal booster on/off
	 */
	ST75256_PWR_CTRL    = 0x20,

	/**
	 * Sets accessible y part of the DDRAM
	 *
	 * It's followed by two bytes.
	 *
	 * 0x00 - 0x28 min y addr
	 * 0x00 - 0x28 max y addr
	 */
	ST75256_SET_PAGE_ADDR = 0x75,

	/**
	 * Sets accessibble x part of the DDRAM
	 *
	 * It's followed by two bytes.
	 *
	 * 0x00 - 0xff min x addr
	 * 0x00 - 0xff max x addr
	 */
	ST75256_SET_COL_ADDR = 0x15,

	/**
	 * Starts writing continous flow of data to DDRAM.
	 */
	ST75256_WRITE_DATA = 0x5c,

	/**
	 * Sets direction DDRAM is scanned when data are displayed.
	 *
	 * Followed by a byte where bits:
	 *
	 * 0x01 Inverses Y direction.
	 * 0x02 Inversed X direction.
	 * 0x04 Sets page direction first otherwise column direction first.
	 */
	ST75256_SET_SCAN_DIRECTION = 0xbc,

	ST75256_SLEEP_OUT   = 0x94, /* Power state sleep out */
	ST75256_SLEEP_IN    = 0x95, /* Power state sleep in */

	/**
	 * Normal display colors 0 = white.
	 */
	ST75256_DISP_NORM   = 0xa6,

	/**
	 * Inverse display colors 0 == black.
	 */
	ST75256_DISP_INV    = 0xa7,

	ST75256_DISP_OFF    = 0xae, /* Display OFF (sleep mode) */
	ST75256_DISP_ON     = 0xaf, /* Display ON in normal mode */

	/**
	 * Display controll, followed by three bytes.
	 *
	 * 0x00 or 0x04 External clock divider
	 * 0x00 - 0xff Duty cycle.
	 * 0x00 - 0x8f
	 */
	ST75256_DISP_CTRL   = 0xca,

	/**
	 * Set LCD Operating Voltage i.e. Contrast. Followed by two bytes.
	 *
	 * 0x00 - 0x3f Upper bytes of the contrast.
	 * 0x00 - 0x07 Lower bytes of the contrast.
	 */
	ST75256_SET_VOP     = 0x81,

	/**
	 * Set display mode. Followed by a byte.
	 *
	 * 0x10 - 1bpp
	 * 0x11 - 2bpp
	 */
	ST75256_DISP_MODE   = 0xf0,

	/* Extension2 commands */

	/**
	 * Sets gray levels, followed by 16 bytes.
	 *
         * The dark and light value is between 0x00 and 0x1f.
	 *
	 * Yes, the dark and light gray bytes are repeated randomly.
	 *
	 * 0x00
	 * 0x00
	 * 0x00
	 * light_gray
	 * light_gray
	 * light_gray
	 * 0x00
	 * 0x00
	 * dark_gray
	 * 0x00
	 * 0x00
	 * dark_gray
	 * dark_gray
	 * dark_gray
	 * 0x00
	 * 0x00
	 */
	ST75256_SET_GRAY_LEVEL = 0x20,

	/**
	 * Sets analog circuts, followed by three data bytes.
	 *
	 * 0x00
	 * 0x00-0x03 Booster efficiency
	 * 0x00-0x07 Bias ratio
	 */
	ST75256_ANALOG_CIRCUIT_SET = 0x32,

	/**
	 * Enables/disables OTP auto read.
	 *
	 * Followed by a single byte.
	 *
	 * 0x8f - enable
	 * 0x9f - disable
	 */
	ST75256_OTP_AUTO_READ_CTRL = 0xd7,

	/**
	 * Enables OTP read/write control.
	 *
	 * Followed by single byte.
	 *
	 * 0x00 enable read
	 * 0x20 enable write
	 */
	ST75256_OTP_WR_RD_CTRL = 0xe0,

	/**
	 * Exits OTP mode.
	 */
	ST75256_OTP_CTRL_OUT = 0xe1,


	/**
	 * Triggers OTP programming.
	 */
	ST75256_OTP_READ = 0xe3,
};


static void st75256_exit(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);

	gp_display_spi_cmd(disp, ST75256_EXT1);
	gp_display_spi_cmd(disp, ST75256_DISP_OFF);
	gp_display_spi_exit(disp);

	free(self->pixmap);
	free(self);
}

/**
 * @param min_x Offset in pixels divided by (8/bpp) 0x00 - 0x28.
 * @param max_x Offset in pixels divided by (8/bpp) min_x - 0x28.
 * @param min_y Offset in pixels 0x00 - 0xff.
 * @param max_y Offset in pixels min_y - 0xff.
 */
static void sel_disp_range(struct gp_display_spi *self,
                           uint8_t min_x, uint8_t max_x,
			   uint8_t min_y, uint8_t max_y)
{
	gp_display_spi_cmd(self, ST75256_SET_COL_ADDR);
	gp_display_spi_data(self, min_y);
	gp_display_spi_data(self, max_y);

	gp_display_spi_cmd(self, ST75256_SET_PAGE_ADDR);
	gp_display_spi_data(self, min_x);
	gp_display_spi_data(self, max_x);
}

/**
 * Sets dark and light gray levels, value is between 0x00 and 0x1f
 */
static void set_gray_level(struct gp_display_spi *self,
                           uint8_t dark_gray, uint8_t light_gray)
{
	gp_display_spi_cmd(self, ST75256_EXT2);
	gp_display_spi_cmd(self, ST75256_SET_GRAY_LEVEL);

	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);

	gp_display_spi_data(self, light_gray);
	gp_display_spi_data(self, light_gray);
	gp_display_spi_data(self, light_gray);

	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);

	gp_display_spi_data(self, dark_gray);

	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);

	gp_display_spi_data(self, dark_gray);
	gp_display_spi_data(self, dark_gray);
	gp_display_spi_data(self, dark_gray);

	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0x00);

	gp_display_spi_cmd(self, ST75256_EXT1);
}

static void read_otp(struct gp_display_spi *self)
{
	gp_display_spi_cmd(self, ST75256_EXT2);

	/* Disable OTP auto read */
	gp_display_spi_cmd_data(self, ST75256_OTP_AUTO_READ_CTRL, 0x9f);

	/* Enable OTP read */
	gp_display_spi_cmd_data(self, ST75256_OTP_WR_RD_CTRL, 0x00);
	usleep(10000);

	/* Load OTP */
	gp_display_spi_cmd(self, ST75256_OTP_READ);
	usleep(20000);

	/* Exit OTP */
	gp_display_spi_cmd(self, ST75256_OTP_CTRL_OUT);
}

static void st75256_init(struct gp_display_spi *disp, uint16_t contrast, uint8_t bpp)
{
	//gp_gpio_write(&disp->gpio_map->pwr, 1);

	/* Reset display */
	gp_gpio_write(&disp->gpio_map->reset, 0);
	usleep(1000);
	gp_gpio_write(&disp->gpio_map->reset, 1);
	usleep(120000);

	/* Wake up the display */
	gp_display_spi_cmd(disp, ST75256_EXT1);
	gp_display_spi_cmd(disp, ST75256_SLEEP_OUT);
	usleep(50000);
	gp_display_spi_cmd(disp, ST75256_DISP_OFF);

	read_otp(disp);

	/* Frame rate 55.5Hz */
	gp_display_spi_cmd(disp, ST75256_EXT2);
	gp_display_spi_cmd(disp, 0xf0);
	gp_display_spi_data(disp, 0x09);
	gp_display_spi_data(disp, 0x09);
	gp_display_spi_data(disp, 0x09);
	gp_display_spi_data(disp, 0x09);

	gp_display_spi_cmd(disp, ST75256_EXT1);

	/* Set column page direction - page direction first & swap X */
	gp_display_spi_cmd_data(disp, ST75256_SET_SCAN_DIRECTION, 0x06);

	/* Inverse display so that 0 is black */
	gp_display_spi_cmd(disp, ST75256_DISP_INV);

	/* Turn on internal voltage generators */
	gp_display_spi_cmd_data(disp, ST75256_PWR_CTRL, 0x0b);
	usleep(1000);

	/* Booster 10x */
	gp_display_spi_cmd_data(disp, 0x51, 0xfb);

	/* Sets contrast */
	gp_display_spi_cmd(disp, ST75256_SET_VOP);
	gp_display_spi_data(disp, contrast & 0x3f);
	gp_display_spi_data(disp, contrast >> 6);

	/* Set gray levels */
	set_gray_level(disp, 0x0f, 0x14);

	gp_display_spi_cmd(disp, ST75256_ANALOG_CIRCUIT_SET);
	gp_display_spi_data(disp, 0x00);
	gp_display_spi_data(disp, 0x01);
	gp_display_spi_data(disp, 0x01);

	/* Set 1bpp or 2bpp mode */
	gp_display_spi_cmd_data(disp, ST75256_DISP_MODE, bpp == 1 ? 0x10 : 0x11);

	gp_display_spi_cmd(disp, ST75256_DISP_CTRL);
	gp_display_spi_data(disp, 0x00);
	gp_display_spi_data(disp, disp->w-1);
	/* Enable line inversion */
	gp_display_spi_data(disp, 0x01);

	gp_display_spi_cmd(disp, ST75256_DISP_ON);
}

static void st75256_2bpp_repaint_full(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	unsigned int y;

	sel_disp_range(disp, 0, disp->w/4-1, 0, disp->h-1);
	gp_display_spi_cmd(disp, ST75256_WRITE_DATA);

	unsigned int row_w = (disp->w+3)/4;

	for (y = 0; y < disp->h; y++) {
		uint8_t *row = &(self->pixmap->pixels[row_w * y]);
		gp_display_spi_data_write(disp, row, row_w);
	}
}

static void st75256_1bpp_repaint_full(gp_backend *self)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	unsigned int y;

	sel_disp_range(disp, 0, disp->w/8-1, 0, disp->h-1);
	gp_display_spi_cmd(disp, ST75256_WRITE_DATA);

	unsigned int row_w = (disp->w + 7)/8;

	for (y = 0; y < disp->h; y++) {
		uint8_t *row = &(self->pixmap->pixels[row_w * y]);

		gp_display_spi_data_write(disp, row, row_w);
	}
}

static void st75256_2bpp_repaint_part(gp_backend *self,
                                      gp_coord x0, gp_coord y0,
                                      gp_coord x1, gp_coord y1)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	/* round the x coordinates to whole bytes */
	unsigned int min_x = (x0)/4;
	unsigned int max_x = (x1+3)/4;
	unsigned int width = max_x - min_x + 1;
	unsigned int row_w = (disp->w+3)/4;
	int y;

	sel_disp_range(disp, min_x, max_x, y0, y1);
	gp_display_spi_cmd(disp, ST75256_WRITE_DATA);

	for (y = y0; y <= y1; y++) {
		uint8_t *row = &(self->pixmap->pixels[row_w * y + min_x]);
		gp_display_spi_data_write(disp, row, width);
	}
}

static void st75256_1bpp_repaint_part(gp_backend *self,
                                      gp_coord x0, gp_coord y0,
                                      gp_coord x1, gp_coord y1)
{
	struct gp_display_spi *disp = GP_BACKEND_PRIV(self);
	/* round the x coordinates to whole bytes */
	unsigned int min_x = x0/8;
	unsigned int max_x = (x1+7)/8;
	unsigned int width = max_x - min_x + 1;
	unsigned int row_w = (disp->w + 7)/8;
	int y;

	sel_disp_range(disp, min_x, max_x, y0, y1);
	gp_display_spi_cmd(disp, ST75256_WRITE_DATA);

	for (y = y0; y <= y1; y++) {
		uint8_t *row = &(self->pixmap->pixels[row_w * y + min_x]);
		gp_display_spi_data_write(disp, row, width);
	}
}

gp_backend *gp_display_st75256_init(const char *conn_id,
                                    uint16_t w, uint16_t h, uint16_t contrast,
                                    unsigned int dpi, uint8_t bpp)
{
	gp_backend *backend;
	struct gp_display_conn *conn;
	int ret;

	switch (bpp) {
	case 1:
	case 2:
		break;
	default:
		GP_FATAL("Invalid bpp %u\n", bpp);
		return NULL;
	};

	if (w > 162 || h > 256) {
		GP_FATAL("Width or height too big %ux%u", w, h);
		return NULL;
	}

	if (contrast > 0x03ff) {
		GP_FATAL("Contrast out of bounds %u > 0x3ff", contrast);
		return NULL;
	}

	conn = gp_display_conn_by_name(conn_id);
	if (!conn)
		return NULL;

	backend = malloc(sizeof(gp_backend) + sizeof(struct gp_display_spi));
	if (!backend) {
		GP_WARN("malloc() failed :(");
		return NULL;
	}

	memset(backend, 0, sizeof(gp_backend) + sizeof(struct gp_display_spi));

	backend->pixmap = gp_pixmap_alloc(w, h, bpp == 1 ? GP_PIXEL_G1_UB : GP_PIXEL_G2_UB);
	if (!backend->pixmap)
		goto err0;

	struct gp_display_spi *disp = GP_BACKEND_PRIV(backend);

	ret = gp_display_spi_init(disp, conn, SPI_MODE_0, 12500000, w, h);
	if (ret)
		goto err1;

	st75256_init(disp, contrast, bpp);

	switch (bpp) {
	case 1:
		backend->update = st75256_1bpp_repaint_full;
		backend->update_rect = st75256_1bpp_repaint_part;
	break;
	case 2:
		backend->update = st75256_2bpp_repaint_full;
		backend->update_rect = st75256_2bpp_repaint_part;
	break;
	}

	backend->exit = st75256_exit;
	backend->dpi = dpi;

	return backend;
err1:
	gp_pixmap_free(backend->pixmap);
err0:
	free(backend);
	return NULL;
}
