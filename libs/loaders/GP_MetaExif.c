/*****************************************************************************
 * This file is part of gfxprim library.                                     *
 *                                                                           *
 * Gfxprim is free software; you can redistribute it and/or                  *
 * modify it under the terms of the GNU Lesser General Public                *
 * License as published by the Free Software Foundation; either              *
 * version 2.1 of the License, or (at your option) any later version.        *
 *                                                                           *
 * Gfxprim is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Lesser General Public License for more details.                           *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public          *
 * License along with gfxprim; if not, write to the Free Software            *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                        *
 * Boston, MA  02110-1301  USA                                               *
 *                                                                           *
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "core/GP_Debug.h"

#include "GP_MetaData.h"

enum IFD_formats {
	/* 1 bytes/components */
	IFD_UNSIGNED_BYTE = 0x01,
	/* 1 bytes/components */
	IFD_ASCII_STRING = 0x02,
	/* 2 bytes/components */
	IFD_UNSIGNED_SHORT = 0x03,
	/* 4 bytes/components */
	IFD_UNSIGNED_LONG = 0x04,
	/* 8 bytes/components */
	IFD_UNSIGNED_RATIONAL = 0x05,
	/* 1 bytes/components */
	IFD_SIGNED_BYTE = 0x06,
	/* 1 bytes/components */
	IFD_UNDEFINED = 0x07,
	/* 2 bytes/components */
	IFD_SIGNED_SHORT = 0x08,
	/* 4 bytes/components */
	IFD_SIGNED_LONG = 0x09,
	/* 8 bytes/components */
	IFD_SIGNED_RATIONAL = 0x0a,
	/* 4 bytes/components */
	IFD_SINGLE_FLOAT = 0x0b,
	/* 8 bytes/components */
	IFD_SINGLE_DOUBLE = 0x0c,
	IFD_FORMAT_LAST = IFD_SINGLE_DOUBLE,
};

static const char *IFD_format_names[] = {
	"Unsigned Byte",
	"ASCII String",
	"Unsigned Short",
	"Unsigned Long",
	"Unsigned Rational",
	"Signed Byte",
	"Undefined",
	"Signed Short",
	"Signed Long",
	"Signed Rational",
	"Single Float",
	"Double Float",
};

enum IFD_tags {
	/* ASCII text no multibyte encoding */
	IFD_IMAGE_DESCRIPTION = 0x010e,
	/* Device (camer, scanner, ...) manufacturer */
	IFD_MAKE = 0x010f,
	/* Device model */
	IFD_MODEL = 0x0110,
	/* Image orientation                           *
	 * 1 upper left, 3 lower right, 6 upper right, *
	 * 8 lower left, other reserved                */
	IFD_ORIENTATION = 0x0112,
	/* X resolution 72 DPI is default */
	IFD_X_RESOLUTION = 0x011a,
	/* Y resolution 72 DPI is default */
	IFD_Y_RESOLUTION = 0x011b,
	/* 1 = no unit, 2 = inch (default), 3 = centimeter */
	IFD_RESOLUTION_UNIT = 0x0128,
	/* Software string. */
	IFD_SOFTWARE = 0x0131,
	/* YYYY:MM:DD HH:MM:SS in 24 hours format */
	IFD_DATE_TIME = 0x0132,
	/* White Point */
	IFD_WHITE_POINT = 0x013e,
	/* Primary Chromaticies */
	IFD_PRIMARY_CHROMATICIES = 0x013f,
	/* YCbCr Coefficients */
	IFD_Y_CB_CR_COEFFICIENTS = 0x0211,
	/* YCbCr Positioning */
	IFD_Y_CB_CR_POSITIONING = 0x0213,
	/* Reference Black White */
	IFD_REFERENCE_BLACK_WHITE = 0x0214,
	/* Copyright */
	IFD_COPYRIGHT = 0x8298,
	/* Exif SubIFD Offset */
	IFD_EXIF_OFFSET = 0x8769,

	/* TAGs from Exif SubIFD */

	IFD_EXPOSURE_TIME = 0x829a,
	/* Actual F-Number of lens when image was taken */
	IFD_F_NUMBER = 0x829d,
	/* 1 manual, 2 normal, 3 aperture priority, 4 shutter priority, *
	 * 5 creative (slow), 6 action (high-speed), 7 portrait mode,   *
	 * 8 landscape mode                                             */
	IFD_EXPOSURE_PROGRAM = 0x8822,
	/* CCD sensitivity */
	IFD_ISO_SPEED_RATINGS = 0x8827,
	/* ASCII 4 byte Exif version */
	IFD_EXIF_VERSION = 0x9000,
	/* Original time should not be modified by user program */
	IFD_DATE_TIME_ORIGINAL = 0x9003,
	IFD_DATE_TIME_DIGITIZED = 0x9004,
	/* Undefined commonly 0x00, 0x01, 0x02, 0x03 */
	IFD_COMPONENT_CONFIGURATION = 0x9101,
	/* Average compression ration */
	IFD_COMPRESSED_BITS_PER_PIXEL = 0x9102,
	/* Shutter speed as 1 / (2 ^ val) */
	IFD_SHUTTER_SPEED_VALUE = 0x9201,
	/* Aperture to convert to F-Number do 1.4142 ^ val */
	IFD_APERTURE_VALUE = 0x9202,
	/* Brightness in EV */
	IFD_BRIGHTNESS_VALUE = 0x9203,
	/* Exposure bias in EV */
	IFD_EXPOSURE_BIAS_VALUE = 0x9204,
	/* Max Aperture in the same format as IFD_APERTURE_VALUE */
	IFD_MAX_APERTURE_VALUE = 0x9205,
	/* Distance to focus point in meters */
	IFD_SUBJECT_DISTANCE = 0x9206,
	/* Exposure metering method, 1 average, 2 center weighted average, *
	 * 3 spot, 4 multi-spot, 5 multi-segment                           */
	IFD_METERING_MODE = 0x9207,
	/* White balance 0 auto, 1 daylight, 2 flourescent, 3 tungsten, 10 flash */
	IFD_LIGHT_SOURCE = 0x9208,
	/* 0 off, 1 on */
	IFD_FLASH = 0x9209,
	/* Focal length in milimeters */
	IFD_FOCAL_LENGTH = 0x920a,
	/* Maker note, undefined may be IFD block */
	IFD_MAKER_NOTE = 0x927c,
	/* Comment */
	IFD_USER_COMMENT = 0x9286,

	/* Stores FlashPix version, undefined, may be four ASCII numbers */
	IFD_FLASH_PIX_VERSION = 0xa000,
	/* Unknown may be 1 */
	IFD_COLOR_SPACE = 0xa001,
	/* Exif Image Width and Height */
	IFD_EXIF_IMAGE_WIDTH = 0xa002,
	IFD_EXIF_IMAGE_HEIGHT = 0xa003,
	/* May store related audio filename */
	IFD_RELATED_SOUND_FILE = 0xa004,
	/* */

};

struct IFD_tag {
	uint16_t tag;
	const char *name;
	uint16_t format;
	/* 0 == not defined */
	uint32_t num_components;
};

/* These are sorted by tag */
static const struct IFD_tag IFD_tags[] = {
	/* TAGs from IFD0 */
	{IFD_IMAGE_DESCRIPTION, "Image Description", IFD_ASCII_STRING, 0},
	{IFD_MAKE, "Make", IFD_ASCII_STRING, 0},
	{IFD_MODEL, "Model", IFD_ASCII_STRING, 0},
	{IFD_ORIENTATION, "Orientation", IFD_UNSIGNED_SHORT, 1},
	{IFD_X_RESOLUTION, "X Resolution", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_Y_RESOLUTION, "Y Resolution", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_RESOLUTION_UNIT, "Resolution Unit", IFD_UNSIGNED_SHORT, 1},
	{IFD_SOFTWARE, "Software", IFD_ASCII_STRING, 0},
	{IFD_DATE_TIME, "Date Time", IFD_ASCII_STRING, 20},
	{IFD_WHITE_POINT, "White Point", IFD_UNSIGNED_RATIONAL, 2},
	{IFD_PRIMARY_CHROMATICIES, "Primary Chromaticies", IFD_UNSIGNED_RATIONAL, 6},
	{IFD_Y_CB_CR_COEFFICIENTS, "YCbCr Conefficients", IFD_UNSIGNED_RATIONAL, 3},
	{IFD_Y_CB_CR_POSITIONING, "YCbCr Positioning", IFD_UNSIGNED_SHORT, 1},
	{IFD_REFERENCE_BLACK_WHITE, "Reference Black White", IFD_UNSIGNED_RATIONAL, 6},
	{IFD_COPYRIGHT, "Copyright", IFD_ASCII_STRING, 0},

	/* TAGs from Exif SubIFD */
	{IFD_EXPOSURE_TIME, "Exposure Time", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_F_NUMBER, "F-Number", IFD_UNSIGNED_RATIONAL, 1},

	/* TAG from IFD0 */
	{IFD_EXIF_OFFSET, "Exif Offset", IFD_UNSIGNED_LONG, 1},

	/* TAGs from Exif SubIFD */
	{IFD_EXPOSURE_PROGRAM, "Exposure Program", IFD_UNSIGNED_SHORT, 1},
	{IFD_ISO_SPEED_RATINGS, "ISO Speed Ratings", IFD_UNSIGNED_SHORT, 1},
	{IFD_EXIF_VERSION, "Exif Version", IFD_UNDEFINED, 4},
	{IFD_DATE_TIME_ORIGINAL, "Date Time Original", IFD_ASCII_STRING, 20},
	{IFD_DATE_TIME_DIGITIZED, "Date Time Digitized", IFD_ASCII_STRING, 20},
	{IFD_COMPONENT_CONFIGURATION, "Component Configuration", IFD_UNDEFINED, 0},
	{IFD_COMPRESSED_BITS_PER_PIXEL, "Compressed Bits Per Pixel", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_SHUTTER_SPEED_VALUE, "Shutter Speed", IFD_SIGNED_RATIONAL, 1},
	{IFD_APERTURE_VALUE, "Aperture", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_BRIGHTNESS_VALUE, "Brightness", IFD_SIGNED_RATIONAL, 1},
	{IFD_EXPOSURE_BIAS_VALUE, "Exposure Bias", IFD_SIGNED_RATIONAL, 1},
	{IFD_MAX_APERTURE_VALUE, "Max Aperture", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_SUBJECT_DISTANCE, "Subject Distance", IFD_SIGNED_RATIONAL, 1},
	{IFD_METERING_MODE, "Metering Mode", IFD_UNSIGNED_SHORT, 1},
	{IFD_LIGHT_SOURCE, "Light Source", IFD_UNSIGNED_SHORT, 1},
	{IFD_FLASH, "Flash", IFD_UNSIGNED_SHORT, 1},
	{IFD_FOCAL_LENGTH, "Focal Length", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_MAKER_NOTE, "Maker Note", IFD_UNDEFINED, 0},
	{IFD_USER_COMMENT, "User Comment", IFD_UNDEFINED, 0},
	{IFD_FLASH_PIX_VERSION, "Flash Pix Version", IFD_UNDEFINED, 4},
	{IFD_COLOR_SPACE, "Color Space", IFD_UNSIGNED_SHORT, 1},
	/* these two may be short in some cases */
	{IFD_EXIF_IMAGE_WIDTH, "Exif Image Width", IFD_UNSIGNED_LONG, 1},
	{IFD_EXIF_IMAGE_HEIGHT, "Exif Image Height", IFD_UNSIGNED_LONG, 1},
	{IFD_RELATED_SOUND_FILE, "Related Soundfile", IFD_ASCII_STRING, 0},
};

static const char *IFD_format_name(uint16_t format)
{
	if (format == 0 || format > IFD_FORMAT_LAST)
		return "Unknown";

	return IFD_format_names[format - 1];
}

static const struct IFD_tag *IFD_tag_get(uint16_t tag)
{
	int left = 0;
	int right = sizeof(IFD_tags)/sizeof(struct IFD_tag) - 1;

	while (right - left > 1) {
		int middle = (right + left)/2;

		if (IFD_tags[middle].tag == tag)
			return &IFD_tags[middle];


		if (IFD_tags[middle].tag > tag)
			right = middle;
		else
			left = middle;
	}

	if (IFD_tags[left].tag == tag)
		return &IFD_tags[left];

	if (IFD_tags[right].tag == tag)
		return &IFD_tags[right];

	return NULL;
}

static const char *IFD_tag_name(uint16_t tag)
{
	const struct IFD_tag *res = IFD_tag_get(tag);

	if (res == NULL)
		return "Unknown";
	else
		return res->name;
}

static int buf_char(void *buf, size_t pos, size_t buf_len)
{
	if (pos >= buf_len) {
		GP_DEBUG(1, "Byte position %zu out of buffer len %zu", pos, buf_len);
		return -1;
	}

	return ((char*)buf)[pos];
}

#define GET_16(res, buf, pos, buf_len, swap) do {   \
	if (pos + 1 >= buf_len) { \
		GP_DEBUG(1, "2-byte position %zu out of buffer len %zu", \
		         (size_t)pos, buf_len); \
		return -1; \
	} \
	\
	if (swap) \
		res = ((uint8_t*)buf)[pos]<<8 | ((uint8_t*)buf)[pos+1]; \
	else \
		res = ((uint8_t*)buf)[pos] | ((uint8_t*)buf)[pos+1]<<8; \
} while (0)

#define GET_32(res, buf, pos, buf_len, swap) do {   \
	if (pos + 3 >= buf_len) { \
		GP_DEBUG(1, "4-byte position %zu out of buffer len %zu", \
		         (size_t)pos, buf_len); \
		return -1; \
	} \
	\
	if (swap) \
		res = (((uint8_t*)buf)[pos])<<24 | (((uint8_t*)buf)[pos+1])<<16 | \
		      (((uint8_t*)buf)[pos+2])<<8 | ((uint8_t*)buf)[pos+3]; \
	else \
		res = ((uint8_t*)buf)[pos] | (((uint8_t*)buf)[pos+1])<<8 | \
		      (((uint8_t*)buf)[pos+2])<<16 | (((uint8_t*)buf)[pos+3])<<24; \
} while (0)

#define GET_16_INC(res, buf, pos, buf_len, swap) do { \
	GET_16(res, buf, pos, buf_len, swap); \
	pos += 2; \
} while (0)

#define GET_32_INC(res, buf, pos, buf_len, swap) do { \
	GET_32(res, buf, pos, buf_len, swap); \
	pos += 4; \
} while (0)

static const char *get_string(void *buf, size_t buf_len,
                              uint32_t num_comp, uint32_t *val)
{
	if (num_comp <= 4)
		return (const char*)val;

	if (*val + num_comp >= buf_len) {
		GP_DEBUG(1, "String out of buffer offset 0x%08x length %u",
		         *val, num_comp);
		return NULL;
	}

	return ((const char*)buf) + *val;
}

static int rat_num(void *buf, uint32_t offset, size_t buf_len, int swap)
{
	int ret;

	GET_32(ret, buf, offset, buf_len, swap);

	return ret;
}

static int rat_den(void *buf, uint32_t offset, size_t buf_len, int swap)
{
	int ret;

	GET_32(ret, buf, offset + 4, buf_len, swap);

	return ret;
}

static void load_tag(GP_MetaData *self, void *buf, size_t buf_len, int swap,
                     uint16_t tag, uint16_t format,
		     uint32_t num_comp, uint32_t val)
{
	const struct IFD_tag *res = IFD_tag_get(tag);

	if (res == NULL) {
		GP_TODO("Skipping unknown IFD tag 0x%02x", tag);
		return;
	}

	if (res->format != format) {
		GP_WARN("Unexpected tag '%s' format '%s' (0x%02x) "
		        "expected '%s'", res->name,
		        IFD_format_name(format), format,
		        IFD_format_name(res->format));
	}

	if ((res->num_components != 0) &&
	    (res->num_components != num_comp)) {
		GP_WARN("Unexpected tag '%s' num_components %u expected %u",
		        res->name, num_comp, res->num_components);
	}

	const char *addr;

	switch (format) {
	case IFD_ASCII_STRING: {
		addr = get_string(buf, buf_len, num_comp, &val);

		if (addr == NULL)
			return;

		GP_MetaDataCreateString(self, res->name, addr, num_comp, 1);
	} break;
	case IFD_UNSIGNED_SHORT:
		if (num_comp == 1)
			GP_MetaDataCreateInt(self, res->name, val);
		else
			goto unused;
	break;
	case IFD_UNSIGNED_RATIONAL:
	case IFD_SIGNED_RATIONAL:
		if (num_comp == 1)
			GP_MetaDataCreateRat(self, res->name,
			                     rat_num(buf, val, buf_len, swap),
					     rat_den(buf, val, buf_len, swap));
		else
			goto unused;
	break;
	case IFD_UNDEFINED:
		switch (res->tag) {
		case IFD_EXIF_VERSION:
		case IFD_FLASH_PIX_VERSION:
			addr = get_string(buf, buf_len, num_comp, &val);

			if (addr == NULL)
				return;

			GP_MetaDataCreateString(self, res->name, addr, num_comp, 1);
		break;
		default:
			goto unused;
		}
	break;
	unused:
	default:
		GP_TODO("Unused record '%s' format '%s' (0x%02x)", res->name,
			IFD_format_name(format), format);
	}
}

/*
 * Loads IFD block.
 */
static int load_IFD(GP_MetaData *self, void *buf, size_t buf_len,
                    uint32_t IFD_offset, int swap)
{
	uint16_t IFD_entries_count;

	GET_16_INC(IFD_entries_count, buf, IFD_offset, buf_len, swap);

	GP_DEBUG(2, "-- IFD Offset 0x%08x Entries 0x%04x --",
	            IFD_offset, IFD_entries_count);

	int i;

	for (i = 0; i < IFD_entries_count; i++) {
		uint16_t tag, format;
		uint32_t num_components, val;

		GET_16_INC(tag, buf, IFD_offset, buf_len, swap);
		GET_16_INC(format, buf, IFD_offset, buf_len, swap);
		GET_32_INC(num_components, buf, IFD_offset, buf_len, swap);
		GET_32_INC(val, buf, IFD_offset, buf_len, swap);

		GP_DEBUG(3, "IFD Entry tag 0x%04x format (0x%04x) components 0x%08x val 0x%08x",
		         tag, format, num_components, val);

		GP_DEBUG(3, "IFD Entry tag '%s' format '%s'",
			 IFD_tag_name(tag), IFD_format_name(format));

		if (tag == IFD_EXIF_OFFSET)
			load_IFD(self, buf, buf_len, val, swap);
		else
			load_tag(self, buf, buf_len, swap, tag, format, num_components, val);
	}
/*
	GET_32(IFD_offset, buf, IFD_offset, buf_len, swap);

	if (IFD_offset != 0x00000000)
		load_IFD(self, buf, buf_len, IFD_offset, swap);
*/
	return 0;
}

/* Offset from the start of the Exit to TIFF header */
#define TIFF_OFFSET 6

int GP_MetaDataFromExif(GP_MetaData *self, void *buf, size_t buf_len)
{
	static int swap = 0;
	int c1, c2;

	if (buf_char(buf, 0, buf_len) != 'E' ||
	    buf_char(buf, 1, buf_len) != 'x' ||
	    buf_char(buf, 2, buf_len) != 'i' ||
	    buf_char(buf, 3, buf_len) != 'f' ||
	    buf_char(buf, 4, buf_len) != 0 ||
	    buf_char(buf, 5, buf_len) != 0) {
		GP_WARN("Missing ASCII 'Exif\\0\\0' string at "
		        "the start of the buffer");
		return 1;
	}

	if (((c1 = buf_char(buf, 6, buf_len)) != 
	    (c2 = buf_char(buf, 7, buf_len)))
	    || (c1 != 'I' && c1 != 'M')) {
		GP_WARN("Expected II or MM got %x%x, corrupt header?", c1, c2);
		return 1;
	}

	swap = (c1 == 'M');

	GP_DEBUG(2, "TIFF aligment is '%c%c' swap = %i", c1, c1, swap);

	uint16_t tag;

	GET_16(tag, buf, 8, buf_len, swap);

	if (tag != 0x002a) {
		GP_WARN("Expected TIFF TAG '0x002a' got '0x%04x'", tag);
		return 1;
	}

	uint32_t IFD_offset;

	GET_32(IFD_offset, buf, 10, buf_len, swap);

	GP_DEBUG(2, "IFD offset is 0x%08x", IFD_offset);

	/* The offset starts from the II or MM */
	load_IFD(self, (char*)buf + TIFF_OFFSET, buf_len - TIFF_OFFSET, IFD_offset, swap);

	return 0;
}
