// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2023 Cyril Hrubis <metan@ucw.cz>
 */

#include <backends/gp_backend.h>
#include <core/gp_pixmap.h>
#include <core/gp_debug.h>

#include "gp_display_eink.h"
#include "gp_display_waveshare.h"
#include "gp_display_ssd16xx.h"

static const uint8_t lut_1bpp_DU[SSD1677_LUT_SIZE] =
{
    /* Source and VCOM voltages */
    0x2A,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//1
    0x05,0x2A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//2
    0x2A,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//3
    0x05,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//4
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//5
    /* 4 x Phase lenght + 1 x Repeat */
    0x00,0x02,0x03,0x0A,0x00,
    0x02,0x06,0x0A,0x05,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    /* Frame rate */
    0x22,0x22,0x22,0x22,0x22
};

static const uint8_t lut_1bpp_A2[SSD1677_LUT_SIZE] =
{
    /* Source and VCOM voltages */
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //1
    0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //2
    0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //3
    0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //4
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //5
    /* 4 x Phase lenght + 1 x Repeat */
    0x00,0x00,0x03,0x05,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,
    /* Frame rate */
    0x22,0x22,0x22,0x22,0x22
};

static const uint8_t lut_ssd168x_part[SSD168X_LUT_SIZE] =
{
    /* Source and VCOM voltages */
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //1
    0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //2
    0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //3
    0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //4
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //5
    /* TP TP SR TP TP SR RP */
    0x00,0x00,0x00, 0x03,0x05,0x00, 0x01,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    0x00,0x00,0x00, 0x00,0x00,0x00, 0x00,
    /* Frame rate */
    0x22,0x22,0x22,0x22,0x22,0x22,
};

static void ssd1677_load_lut(struct gp_display_spi *self, const uint8_t *lut)
{
	GP_DEBUG(4, "Writing LUT");
	gp_display_spi_cmd(self, SSD16XX_WRITE_LUT);
	gp_display_spi_data_transfer(self, lut, NULL, SSD1677_LUT_SIZE);
	gp_display_spi_wait_ready(self, 0);
}

static void ssd168x_load_lut(struct gp_display_spi *self, const uint8_t *lut)
{
	GP_DEBUG(4, "Writing LUT");
	gp_display_spi_cmd(self, SSD16XX_WRITE_LUT);
	gp_display_spi_data_transfer(self, lut, NULL, SSD168X_LUT_SIZE);
	gp_display_spi_wait_ready(self, 0);
}

static void ssd1677_set_ram_addr(struct gp_display_spi *spi, uint16_t x, uint16_t y)
{
	GP_DEBUG(3, "Setting RAM start %u x %u", x, y);

	gp_display_spi_cmd(spi, SSD16XX_XRAM_ADDR);
	gp_display_spi_data(spi, x & 0xff);
	gp_display_spi_data(spi, (x >> 8) & 0x03);

	gp_display_spi_cmd(spi, SSD16XX_YRAM_ADDR);
	gp_display_spi_data(spi, y & 0xff);
	gp_display_spi_data(spi, (y >> 8) & 0x03);
}

static void ssd1677_set_ram_window(struct gp_display_spi *spi,
                                   uint16_t x1, uint16_t x2,
                                   uint16_t y1, uint16_t y2)
{
	GP_DEBUG(3, "Setting RAM window %u-%u x %u-%u", x1, x2, y1, y2);

	gp_display_spi_cmd(spi, SSD16XX_XRAM_RANGE);
	gp_display_spi_data(spi, x1 & 0xff);
	gp_display_spi_data(spi, (x1 >> 8) & 0x03);
	gp_display_spi_data(spi, x2 & 0xff);
	gp_display_spi_data(spi, (x2 >> 8) & 0x03);

	gp_display_spi_cmd(spi, SSD16XX_YRAM_RANGE);
	gp_display_spi_data(spi, y1 & 0xff);
	gp_display_spi_data(spi, (y1 >> 8) & 0x03);
	gp_display_spi_data(spi, y2 & 0xff);
	gp_display_spi_data(spi, (y2 >> 8) & 0x03);
}

static void ssd1677_reset_ram_window(struct gp_display_spi *spi)
{
	ssd1677_set_ram_window(spi, 0, spi->w-1, 0, spi->h-1);
}

static void ssd168x_set_ram_addr(struct gp_display_spi *spi, uint16_t x, uint16_t y)
{
	GP_DEBUG(3, "Setting RAM start %u x %u", x, y);

	gp_display_spi_cmd(spi, SSD16XX_XRAM_ADDR);
	gp_display_spi_data(spi, (x/8) & 0x3f);

	gp_display_spi_cmd(spi, SSD16XX_YRAM_ADDR);
	gp_display_spi_data(spi, y & 0xff);
	gp_display_spi_data(spi, (y >> 8) & 0x01);
}

static void ssd168x_set_ram_window(struct gp_display_spi *spi,
                                   uint16_t x1, uint16_t x2,
                                   uint16_t y1, uint16_t y2)
{
	GP_DEBUG(3, "Setting RAM window %u-%u x %u-%u", x1, x2, y1, y2);

	gp_display_spi_cmd(spi, SSD16XX_XRAM_RANGE);
	gp_display_spi_data(spi, (x1/8) & 0x3f);
	gp_display_spi_data(spi, (x2/8) & 0x3f);

	gp_display_spi_cmd(spi, SSD16XX_YRAM_RANGE);
	gp_display_spi_data(spi, y1 & 0xff);
	gp_display_spi_data(spi, (y1 >> 8) & 0x01);
	gp_display_spi_data(spi, y2 & 0xff);
	gp_display_spi_data(spi, (y2 >> 8) & 0x01);
}

static void ssd168x_reset_ram_window(struct gp_display_spi *spi)
{
	ssd168x_set_ram_window(spi, 0, spi->w-1, 0, spi->h-1);
}

static void ssd16xx_clear_red_ram(struct gp_display_spi *spi)
{
	gp_display_spi_cmd(spi, SSD16XX_CLR_RED_RAM);
	gp_display_spi_data(spi, 0xf7);
	gp_display_spi_wait_ready(spi, 0);
}

static void ssd16xx_clear_bw_ram(struct gp_display_spi *spi)
{
	gp_display_spi_cmd(spi, SSD16XX_CLR_BW_RAM);
	gp_display_spi_data(spi, 0xf7);
	gp_display_spi_wait_ready(spi, 0);
}

static void ssd16xx_pwr_on_and_reset(struct gp_display_spi *disp)
{
	GP_DEBUG(4, "Turning on display power + hardware and software reset");

	gp_gpio_write(&disp->gpio_map->reset, 0);
	usleep(1000);
	gp_gpio_write(&disp->gpio_map->pwr, 1);
	usleep(10000);
	gp_gpio_write(&disp->gpio_map->reset, 1);
	usleep(10000);
	gp_display_spi_cmd(disp, SSD16XX_SW_RESET);
	usleep(10000);
}

/*
 * Sets the gates accordingly to the display pixel height.
 *
 * The first two bytes is the last y coordinate i.e. height-1.
 */
static void ssd16xx_doc(struct gp_display_spi *disp, uint16_t last_y)
{
	gp_display_spi_cmd(disp, SSD16XX_DOC);
	gp_display_spi_data(disp, (last_y & 0xff));
	gp_display_spi_data(disp, (last_y >> 8));
	gp_display_spi_data(disp, 0x00);
}

/*
 * Reads build-in temp sensor.
 */
static void ssd16xx_read_temp(struct gp_display_spi *disp)
{
	gp_display_spi_cmd(disp, SSD16XX_TEMP_CTRL);
	gp_display_spi_data(disp, 0x80);
}

static void ssd16xx_data_entry_mode(struct gp_display_spi *disp, uint8_t dem)
{
	gp_display_spi_cmd(disp, SSD16XX_DEM);
	gp_display_spi_data(disp, dem);
}

static void ssd168x_init(struct gp_display_spi *disp)
{
	ssd16xx_pwr_on_and_reset(disp);
	ssd16xx_clear_red_ram(disp);
	ssd16xx_clear_bw_ram(disp);
	ssd16xx_doc(disp, disp->h - 1);
	ssd16xx_read_temp(disp);
	ssd168x_reset_ram_window(disp);
	ssd168x_set_ram_addr(disp, 0, 0);

	ssd16xx_data_entry_mode(disp, 0x03);
}

static void waveshare_3_7_init(struct gp_display_spi *self)
{
	ssd16xx_pwr_on_and_reset(self);

	ssd16xx_clear_red_ram(self);
	ssd16xx_clear_bw_ram(self);

	ssd16xx_doc(self, self->h - 1);

	gp_display_spi_cmd(self, SSD16XX_BSST);
	gp_display_spi_data(self, 0xae);
	gp_display_spi_data(self, 0xc7);
	gp_display_spi_data(self, 0xc3);
	gp_display_spi_data(self, 0xc0);
	gp_display_spi_data(self, 0xc0);

	ssd16xx_read_temp(self);

	/* Display Mode */
	gp_display_spi_cmd(self, SSD16XX_WRDO);
	gp_display_spi_data(self, 0x00);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0x4f);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);
	gp_display_spi_data(self, 0xff);

	ssd1677_reset_ram_window(self);

	gp_display_spi_cmd(self, SSD16XX_UPDT_CTRL2);
	gp_display_spi_data(self, SSD16XX_UPDT_EN_CLK | SSD16XX_UPDT_EN_ANALOG |
	                          SSD16XX_UPDT_DISPLAY |
	                          SSD16XX_UPDT_DIS_ANALOG | SSD16XX_UPDT_DIS_CLK);
}

static void ssd16xx_deep_sleep(struct gp_display_spi *self)
{
	GP_DEBUG(4, "Entering deep sleep");
	gp_display_spi_cmd(self, SSD16XX_DSLP);
	gp_display_spi_cmd(self, 0x03);
	usleep(10000);
}

static void ssd16xx_display_exit(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);

	ssd16xx_deep_sleep(&eink->spi);
	gp_display_spi_exit(&eink->spi);

	free(self->pixmap);
	free(self);
}

static void ssd168x_full_repaint_start(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *disp = &eink->spi;
	unsigned int y;

	ssd168x_reset_ram_window(disp);
	ssd168x_set_ram_addr(disp, 0, 0);

	gp_display_spi_cmd(disp, SSD16XX_WRITE_BW_RAM);

	uint16_t line_bytes = (disp->w + 0x07)/8;

	for (y = 0; y < disp->h; y++) {
		uint8_t *tx_buf = &self->pixmap->pixels[line_bytes * y];
		gp_display_spi_data_transfer(disp, tx_buf, NULL, line_bytes);
	}

	gp_display_spi_cmd(disp, SSD16XX_UPDT_CTRL2);
	gp_display_spi_data(disp, SSD16XX_UPDT_EN_CLK | SSD16XX_UPDT_EN_ANALOG |
				  SSD16XX_UPDT_READ_TEMP | SSD16XX_UPDT_LOAD_LUT |
	                          SSD16XX_UPDT_DISPLAY |
	                          SSD16XX_UPDT_DIS_ANALOG | SSD16XX_UPDT_DIS_CLK);

	gp_display_spi_busy_edge_set(disp, GP_GPIO_EDGE_FALL);
	gp_display_spi_cmd(disp, SSD16XX_DISP_UPDT);
}

static void repaint_full_start(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;
	unsigned int y;

	ssd1677_reset_ram_window(spi);

	ssd1677_set_ram_addr(spi, 0, 0);

	gp_display_spi_cmd(spi, SSD16XX_WRITE_BW_RAM);

	uint16_t line_bytes = spi->w/8;

	for (y = 0; y < spi->h; y++) {
		uint8_t *tx_buf = &self->pixmap->pixels[line_bytes * y];

		gp_display_spi_data_transfer(spi, tx_buf, NULL, line_bytes);
	}

	ssd1677_load_lut(spi, lut_1bpp_DU);
	gp_display_spi_busy_edge_set(spi, GP_GPIO_EDGE_FALL);
	gp_display_spi_cmd(spi, SSD16XX_DISP_UPDT);
}

static void ssd16xx_repaint_finish(gp_backend *self)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *disp = &eink->spi;

	gp_display_spi_busy_edge_set(disp, GP_GPIO_EDGE_NONE);
}

static void repaint_part_start(gp_backend *self,
                               gp_coord x0, gp_coord y0,
                               gp_coord x1, gp_coord y1)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *spi = &eink->spi;

	uint16_t x_start = x0 & ~0x07;
	uint16_t x_end = (x1 + 0x06) & ~0x07;
	uint16_t y_start = y0;
	uint16_t y_end = y1;
	unsigned int y;

	ssd1677_set_ram_window(spi, x_start, x_end, y_start, y_end);
	ssd1677_set_ram_addr(spi, x_start, y_start);

	gp_display_spi_cmd(spi, SSD16XX_WRITE_BW_RAM);

	unsigned int line_bytes = spi->w/8;
	unsigned int len = (x_end - x_start)/8  + 1;

	for (y = y_start; y <= y_end; y++) {
		uint8_t *tx_buf = &self->pixmap->pixels[line_bytes * y + x_start/8];

		gp_display_spi_data_transfer(spi, tx_buf, NULL, len);
	}

	ssd1677_load_lut(spi, lut_1bpp_A2);
	gp_display_spi_busy_edge_set(spi, GP_GPIO_EDGE_FALL);
	gp_display_spi_cmd(spi, SSD16XX_DISP_UPDT);
}

static void ssd168x_part_repaint_start(gp_backend *self,
                                       gp_coord x0, gp_coord y0,
                                       gp_coord x1, gp_coord y1)
{
	struct gp_display_eink *eink = GP_BACKEND_PRIV(self);
	struct gp_display_spi *disp = &eink->spi;
	unsigned int y;

	uint16_t xs = x0 & ~0x07;
	uint16_t xe = (x1 + 0x06) & ~0x07;
	uint16_t ys = y0;
	uint16_t ye = y1;

	ssd168x_set_ram_window(disp, xs, xe, ys, ye);
	ssd168x_set_ram_addr(disp, xs, ys);

	gp_display_spi_cmd(disp, SSD16XX_WRITE_BW_RAM);

	unsigned int line_bytes = (disp->w + 0x06)/8;
	unsigned int len = (xe - xs)/8 + 1;

	for (y = ys; y <= ye; y++) {
		uint8_t *tx_buf = &self->pixmap->pixels[line_bytes * y + xs/8];

		gp_display_spi_data_transfer(disp, tx_buf, NULL, len);
	}

	gp_display_spi_cmd(disp, SSD16XX_UPDT_CTRL2);
	gp_display_spi_data(disp, SSD16XX_UPDT_EN_CLK | SSD16XX_UPDT_EN_ANALOG |
	                          SSD16XX_UPDT_DISPLAY |
	                          SSD16XX_UPDT_DIS_ANALOG | SSD16XX_UPDT_DIS_CLK);
	ssd168x_load_lut(disp, lut_ssd168x_part);
	gp_display_spi_busy_edge_set(disp, GP_GPIO_EDGE_FALL);
	gp_display_spi_cmd(disp, SSD16XX_DISP_UPDT);
}

static gp_backend *ssd16xx_backend_init(uint16_t w, uint16_t h, gp_pixel_type pixel_type)
{
	gp_backend *backend;
	int ret;

	backend = malloc(sizeof(gp_backend) + sizeof(struct gp_display_eink));
	if (!backend) {
		GP_WARN("malloc() failed :(");
		return NULL;
	}

	memset(backend, 0, sizeof(gp_backend) + sizeof(struct gp_display_eink));

	backend->pixmap = gp_pixmap_alloc(w, h, pixel_type);
	if (!backend->pixmap)
		goto err0;

	struct gp_display_eink *eink = GP_BACKEND_PRIV(backend);

	ret = gp_display_spi_init(&eink->spi, EINK_SPI_DEV, EINK_SPI_MODE,
	                          EINK_SPI_SPEED_HZ, &gpio_map_rpi, w, h);
	if (ret)
		goto err1;

	eink->display_exit = ssd16xx_display_exit;

	return backend;
err1:
	gp_pixmap_free(backend->pixmap);
err0:
	free(backend);
	return NULL;
}

gp_backend *gp_waveshare_3_7_init(void)
{
	gp_backend *backend;

	backend = ssd16xx_backend_init(280, 480, GP_PIXEL_G1);
	if (!backend)
		return NULL;

	struct gp_display_eink *eink = GP_BACKEND_PRIV(backend);

	waveshare_3_7_init(&eink->spi);

	eink->full_repaint_ms = 1000;
	eink->part_repaint_ms = 500;
	eink->repaint_full_start = repaint_full_start;
	eink->repaint_full_finish = ssd16xx_repaint_finish;
	eink->repaint_part_start = repaint_part_start;
	eink->repaint_part_finish = ssd16xx_repaint_finish;

	gp_display_eink_init(backend);

	backend->dpi = 150;

	return backend;
}

gp_backend *gp_weact_2_13_init(void)
{
	gp_backend *backend;

	backend = ssd16xx_backend_init(122, 250, GP_PIXEL_G1);
	if (!backend)
		return NULL;

	struct gp_display_eink *eink = GP_BACKEND_PRIV(backend);

	ssd168x_init(&eink->spi);

	eink->repaint_full_start = ssd168x_full_repaint_start;
	eink->repaint_full_finish = ssd16xx_repaint_finish;
	eink->repaint_part_start = ssd168x_part_repaint_start;
	eink->repaint_part_finish = ssd16xx_repaint_finish;

	gp_display_eink_init(backend);

	backend->dpi = 130;

	return backend;
}
