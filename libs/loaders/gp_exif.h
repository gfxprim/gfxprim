// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 *
 */

enum IFD_EXIF_tags {
	/* Stored in interop IFD */
	IFD_INTEROP_VERSION = 0x2,

	/* Image width and height */
	IFD_IMAGE_WIDTH = 0x0100,
	IFD_IMAGE_HEIGHT = 0x0101,

	IFD_BITS_PER_SAMPLE = 0x0102,
	/* TODO: enum of compressions */
	IFD_COMPRESSION = 0x0103,
	/* TODO: enum of interpretations */
	IFD_PHOTOMETRIC_INTERPRETATION = 0x0106,

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
	/* 1 = Chunky, 2 = Planar */
	IFD_PLANAR_CONFIGURATION = 0x011c,
	/* 1 = No unit, 2 = Inch (default), 3 = Centimeter */
	IFD_RESOLUTION_UNIT = 0x0128,
	/* Software string. */
	IFD_SOFTWARE = 0x0131,
	/* YYYY:MM:DD HH:MM:SS in 24 hours format */
	IFD_DATE_TIME = 0x0132,
	/* Artist */
	IFD_ARTIST = 0x013b,
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
	/* Stored in interop IFD */
	IFD_RELATED_IMAGE_WIDTH = 0x1001,
	IFD_RELATED_IMAGE_HEIGHT = 0x1002,
	/* Copyright */
	IFD_COPYRIGHT = 0x8298,
	IFD_EXPOSURE_TIME = 0x829a,
	/* Actual F-Number of lens when image was taken */
	IFD_F_NUMBER = 0x829d,
	/* Exif SubIFD Offset */
	IFD_EXIF_OFFSET = 0x8769,
	/* 1 manual, 2 normal, 3 aperture priority, 4 shutter priority, *
	 * 5 creative (slow), 6 action (high-speed), 7 portrait mode,   *
	 * 8 landscape mode                                             */
	IFD_EXPOSURE_PROGRAM = 0x8822,
	/* Offset to GPS data */
	IFD_GPS_OFFSET = 0x8825,
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

	/* Subsec time in addition to IFD_DATE_TIME* */
	IFD_SUBSEC_TIME = 0x9290,
	IFD_SUBSEC_TIME_ORIGINAL = 0x9291,
	IFD_SUBSEC_TIME_DIGITIZED = 0x9292,

	/* Stores FlashPix version, undefined, may be four ASCII numbers */
	IFD_FLASH_PIX_VERSION = 0xa000,
	/* Unknown may be 1 */
	IFD_COLOR_SPACE = 0xa001,
	/* Exif Image Width and Height */
	IFD_EXIF_IMAGE_WIDTH = 0xa002,
	IFD_EXIF_IMAGE_HEIGHT = 0xa003,
	/* May store related audio filename */
	IFD_RELATED_SOUND_FILE = 0xa004,
	/* Offset to Interoperability SubIFD */
	IFD_INTEROPERABILITY_OFFSET = 0xa005,
	/* */
	IFD_FOCAL_PLANE_X_RESOLUTION = 0xa20e,
	IFD_FOCAL_PLANE_Y_RESOLUTION = 0xa20f,
	/* 1 = No unit, 2 = Inch (default), 3 = Centimeter */
	IFD_FOCAL_PLANE_RESOLUTION_UNIT = 0xa210,
	/* TODO: enum of sensing methods */
	IFD_SENSING_METHOD = 0xa217,
	IFD_FILE_SOURCE = 0xa300,
	/* Set to 1 when image was directly photographed */
	IFD_SCENE_TYPE = 0xa301,
	/* Color Filter Array pattern */
	IFD_CFA_PATTERN = 0xa302,
	/* 0 = Normal, 1 = Custom */
	IFD_CUSTOM_RENDERER = 0xa401,
	/* 0 = Auto, 1 = Manual, 2 = Auto bracket */
	IFD_EXPOSURE_MODE = 0xa402,
	/* 0 = Auto, 1 = Manual */
	IFD_WHITE_BALANCE = 0xa403,
	/* 0 == Not Used */
	IFD_DIGITAL_ZOOM_RATIO = 0xa404,
	/* Equivalent lenght assuming 35mm camera */
	IFD_FOCAL_LENGTH_IN_35_MM_FILM = 0xa405,
	/* 0 = Standard, 1 = Landscape, 2 = Portrait, 3 = Night Scene */
	IFD_SCENE_CAPTURE_TYPE = 0xa406,
	/* 0 = None, 1 = Low Gain Up, 2 = High Gain Up, 3 = Low Gain Down, 4 = High Gain Down */
	IFD_GAIN_CONTROL = 0xa407,
	/* 0 = Normal, 1 = Soft, 2 = Hard */
	IFD_CONTRAST = 0xa408,
	/* 0 = Normal, 1 = Low Saturation, 2 = Hight Saturation */
	IFD_SATURATION = 0xa409,
	/* 0 = Normal, 1 = Sort, 2 = Hard */
	IFD_SHARPNESS = 0xa40a,
	/* 0 = Unknown, 1 = Macro, 2 = Close View, 3 = Distant View */
	IFD_SUBJECT_DISTANCE_RANGE = 0xa40c,
	/* Unique in hex string */
	IFD_IMAGE_UNIQUE_ID = 0xa420,

	/* Camera owner name - string */
	IFD_CAMERA_OWNER_NAME = 0xa430,
	/* Body serial number - string */
	IFD_BODY_SERIAL_NUMBER = 0xa431,
	/* Lens specification - rational */
	IFD_LENS_SPECIFICATION = 0xa432,
	/* Lens maker - string */
	IFD_LENS_MAKE = 0xa433,
	/* Lens model - string */
	IFD_LENS_MODEL = 0xa434,
	/* Lens type - string */
	IFD_LENS_SERIAL_NUMBER = 0xa435,

	/* Rational */
	IFD_IMAGE_GAMMA = 0xa500,

	/* ??? */
	IFD_PRINT_IM = 0xc4a5,

	/* Padding */
	IFD_PADDING = 0xea1c,
	/* Microsoft's ill-conceived maker note offset difference */
	IFD_OFFSET_SCHEMA = 0xea1d,
};

/* MUST SORTED BY TAG */
static const struct IFD_tag IFD_EXIF_taglist[] = {
	/* Stored in interop IFD */
	{IFD_INTEROP_VERSION, "Interop Version", IFD_UNDEFINED, 4},

	/* TODO May be LONG */
	{IFD_IMAGE_WIDTH, "Image Width", IFD_UNSIGNED_SHORT, 1},
	{IFD_IMAGE_HEIGHT, "Image Height", IFD_UNSIGNED_SHORT, 1},

	{IFD_BITS_PER_SAMPLE, "Bits Per Sample", IFD_UNSIGNED_SHORT, 3},
	{IFD_COMPRESSION, "Compression", IFD_UNSIGNED_SHORT, 1},
	{IFD_PHOTOMETRIC_INTERPRETATION, "Photometric Interpretation", IFD_UNSIGNED_SHORT, 1},

	{IFD_IMAGE_DESCRIPTION, "Image Description", IFD_ASCII_STRING, 0},
	{IFD_MAKE, "Make", IFD_ASCII_STRING, 0},
	{IFD_MODEL, "Model", IFD_ASCII_STRING, 0},
	{IFD_ORIENTATION, "Orientation", IFD_UNSIGNED_SHORT, 1},
	{IFD_X_RESOLUTION, "X Resolution", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_Y_RESOLUTION, "Y Resolution", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_PLANAR_CONFIGURATION, "Planar Configuration", IFD_UNSIGNED_SHORT, 1},
	{IFD_RESOLUTION_UNIT, "Resolution Unit", IFD_UNSIGNED_SHORT, 1},
	{IFD_SOFTWARE, "Software", IFD_ASCII_STRING, 0},
	{IFD_DATE_TIME, "Date Time", IFD_ASCII_STRING, 0},
	{IFD_ARTIST, "Artist", IFD_ASCII_STRING, 0},
	{IFD_WHITE_POINT, "White Point", IFD_UNSIGNED_RATIONAL, 2},
	{IFD_PRIMARY_CHROMATICIES, "Primary Chromaticies", IFD_UNSIGNED_RATIONAL, 6},
	{IFD_Y_CB_CR_COEFFICIENTS, "YCbCr Conefficients", IFD_UNSIGNED_RATIONAL, 3},
	{IFD_Y_CB_CR_POSITIONING, "YCbCr Positioning", IFD_UNSIGNED_SHORT, 1},
	{IFD_REFERENCE_BLACK_WHITE, "Reference Black White", IFD_UNSIGNED_RATIONAL, 6},

	/* Stored in interop IFD */
	{IFD_RELATED_IMAGE_WIDTH, "Related Image Width", IFD_UNSIGNED_LONG, 1},
	{IFD_RELATED_IMAGE_HEIGHT, "Related Image Height", IFD_UNSIGNED_LONG, 1},

	{IFD_COPYRIGHT, "Copyright", IFD_ASCII_STRING, 0},

	/* TAGs from Exif SubIFD */
	{IFD_EXPOSURE_TIME, "Exposure Time", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_F_NUMBER, "F-Number", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_EXIF_OFFSET, "Exif Offset", IFD_UNSIGNED_LONG, 1},
	{IFD_EXPOSURE_PROGRAM, "Exposure Program", IFD_UNSIGNED_SHORT, 1},
	{IFD_GPS_OFFSET, "GPS Offset", IFD_UNSIGNED_LONG, 1},
	{IFD_ISO_SPEED_RATINGS, "ISO Speed Ratings", IFD_UNSIGNED_SHORT, 1},
	{IFD_EXIF_VERSION, "Exif Version", IFD_UNDEFINED, 4},
	{IFD_DATE_TIME_ORIGINAL, "Date Time Original", IFD_ASCII_STRING, 0},
	{IFD_DATE_TIME_DIGITIZED, "Date Time Digitized", IFD_ASCII_STRING, 0},
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
	{IFD_SUBSEC_TIME, "Subsec Time", IFD_ASCII_STRING, 0},
	{IFD_SUBSEC_TIME_ORIGINAL, "Subsec Time Original", IFD_ASCII_STRING, 0},
	{IFD_SUBSEC_TIME_DIGITIZED, "Subsec Time Digitized", IFD_ASCII_STRING, 0},
	{IFD_FLASH_PIX_VERSION, "Flash Pix Version", IFD_UNDEFINED, 4},
	{IFD_COLOR_SPACE, "Color Space", IFD_UNSIGNED_SHORT, 1},
	/* these two may be short in some cases */
	{IFD_EXIF_IMAGE_WIDTH, "Exif Image Width", IFD_UNSIGNED_LONG, 1},
	{IFD_EXIF_IMAGE_HEIGHT, "Exif Image Height", IFD_UNSIGNED_LONG, 1},
	{IFD_RELATED_SOUND_FILE, "Related Soundfile", IFD_ASCII_STRING, 0},
	{IFD_INTEROPERABILITY_OFFSET, "Interoperability Offset", IFD_UNSIGNED_LONG, 1},
	{IFD_FOCAL_PLANE_X_RESOLUTION, "Focal Plane X Resolution", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_FOCAL_PLANE_Y_RESOLUTION, "Focal Plane Y Resolution", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_FOCAL_PLANE_RESOLUTION_UNIT, "Focal Plane Resolution Unit", IFD_UNSIGNED_SHORT, 1},
	{IFD_SENSING_METHOD, "Sensing Method", IFD_UNSIGNED_SHORT, 1},
	{IFD_FILE_SOURCE, "File Source", IFD_UNDEFINED, 1},
	{IFD_SCENE_TYPE, "Scene Type", IFD_UNDEFINED, 1},
	{IFD_CFA_PATTERN, "CFA Pattern", IFD_UNDEFINED, 0},
	{IFD_CUSTOM_RENDERER, "Custom Renderer", IFD_UNSIGNED_SHORT, 1},
	{IFD_EXPOSURE_MODE, "Exposure Mode", IFD_UNSIGNED_SHORT, 1},
	{IFD_WHITE_BALANCE, "White Balance", IFD_UNSIGNED_SHORT, 1},
	{IFD_DIGITAL_ZOOM_RATIO, "Digital Zoom Ratio", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_FOCAL_LENGTH_IN_35_MM_FILM, "Focal Lenght In 35mm Film", IFD_UNSIGNED_SHORT, 1},
	{IFD_SCENE_CAPTURE_TYPE, "Scene Capture Type", IFD_UNSIGNED_SHORT, 1},
	{IFD_GAIN_CONTROL, "Gain Control", IFD_UNSIGNED_SHORT, 1},
	{IFD_CONTRAST, "Contrast", IFD_UNSIGNED_SHORT, 1},
	{IFD_SATURATION, "Saturation", IFD_UNSIGNED_SHORT, 1},
	{IFD_SHARPNESS, "Sharpness", IFD_UNSIGNED_SHORT, 1},
	{IFD_SUBJECT_DISTANCE_RANGE, "Subject Distance Range", IFD_UNSIGNED_SHORT, 1},
	{IFD_IMAGE_UNIQUE_ID, "Image Unique ID", IFD_ASCII_STRING, 33},

	{IFD_CAMERA_OWNER_NAME, "Camera Owner Name", IFD_ASCII_STRING, 0},
	{IFD_BODY_SERIAL_NUMBER, "Body Serial Number", IFD_ASCII_STRING, 0},
	{IFD_LENS_SPECIFICATION, "Lens Specification", IFD_UNSIGNED_RATIONAL, 4},
	{IFD_LENS_MAKE, "Lens Make", IFD_ASCII_STRING, 0},
	{IFD_LENS_MODEL, "Lens Model", IFD_ASCII_STRING, 0},
	{IFD_LENS_SERIAL_NUMBER, "Lens Serial Number", IFD_ASCII_STRING, 0},

	{IFD_IMAGE_GAMMA, "Image gamma", IFD_UNSIGNED_RATIONAL, 1},

	{IFD_PRINT_IM, "Print IM", IFD_UNDEFINED, 0},
	{IFD_PADDING, "Padding", IFD_UNDEFINED, 0},
	{IFD_OFFSET_SCHEMA, "Offset Schema", IFD_SIGNED_LONG, 1},
};

static const struct IFD_tags IFD_EXIF_tags = {
	.tags = IFD_EXIF_taglist,
	.tag_cnt = GP_ARRAY_SIZE(IFD_EXIF_taglist),
	.id = "Exif",
};
