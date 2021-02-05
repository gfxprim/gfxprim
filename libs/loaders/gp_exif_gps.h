// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 *
 */

enum IFD_GPS_tags {
	IFD_GPS_VERSION_ID = 0x0,
	/* 'N' = North, 'S' = South */
	IFD_GPS_LATITUDE_REF = 0x1,
	/* Degrees, Minutes, Seconds */
	IFD_GPS_LATITUDE = 0x2,
	/* 'E' = East, 'W' = West */
	IFD_GPS_LONGITUDE_REF = 0x3,
	/* Degrees, Minutes, Seconds */
	IFD_GPS_LONGITUDE = 0x4,
	/* 0 = Sea level, 1 = Sea level reference (negative value) */
	IFD_GPS_ALTITUDE_REF = 0x5,
	/* Altitude in meters */
	IFD_GPS_ALTITUDE = 0x6,
	/* Hour, Minute, Second */
	IFD_GPS_TIMESTAMP = 0x7,

	/* 'TOKYO' or 'WGS-84' */
	IFD_GPS_MAP_DATUM = 0x12,
};

/* MUST SORTED BY TAG */
static const struct IFD_tag IFD_GPS_taglist[] = {
	{IFD_GPS_VERSION_ID, "Version Info", IFD_UNSIGNED_BYTE, 4},
	{IFD_GPS_LATITUDE_REF, "Latitude Ref", IFD_ASCII_STRING, 2},
	{IFD_GPS_LATITUDE, "Latitude", IFD_UNSIGNED_RATIONAL, 3},
	{IFD_GPS_LONGITUDE_REF, "Longitude Ref", IFD_ASCII_STRING, 2},
	{IFD_GPS_LONGITUDE, "Longitude", IFD_UNSIGNED_RATIONAL, 3},
	{IFD_GPS_ALTITUDE_REF, "Altitude Ref", IFD_UNSIGNED_BYTE, 1},
	{IFD_GPS_ALTITUDE, "Altitude", IFD_UNSIGNED_RATIONAL, 1},
	{IFD_GPS_TIMESTAMP, "Time Stamp", IFD_UNSIGNED_RATIONAL, 3},

	{IFD_GPS_MAP_DATUM, "Map Datum", IFD_ASCII_STRING, 0},
};

static const struct IFD_tags IFD_GPS_tags = {
	.tags = IFD_GPS_taglist,
	.tag_cnt = GP_ARRAY_SIZE(IFD_GPS_taglist),
	.id = "GPS",
};
