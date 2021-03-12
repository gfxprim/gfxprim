// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2014 Cyril Hrubis <metan@ucw.cz>
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "core/gp_common.h"
#include <core/gp_debug.h>
#include <loaders/gp_exif.h>

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

static const char *IFD_format_name(uint16_t format)
{
	if (format == 0 || format > IFD_FORMAT_LAST)
		return "Unknown";

	return IFD_format_names[format - 1];
}

struct IFD_tag {
	uint16_t tag;
	const char *name;
	uint16_t format;
	/* 0 == not defined */
	uint32_t num_components;
};

struct IFD_tags {
	const struct IFD_tag *tags;
	unsigned int tag_cnt;
	const char *id;
};

static const struct IFD_tag *IFD_tag_get(const struct IFD_tags *tags,
                                         uint16_t tag)
{
	int left = 0;
	int right = tags->tag_cnt - 1;

	while (right - left > 1) {
		int middle = (right + left)/2;

		if (tags->tags[middle].tag == tag)
			return &tags->tags[middle];


		if (tags->tags[middle].tag > tag)
			right = middle;
		else
			left = middle;
	}

	if (tags->tags[left].tag == tag)
		return &tags->tags[left];

	if (tags->tags[right].tag == tag)
		return &tags->tags[right];

	return NULL;
}

static const char *IFD_tag_name(const struct IFD_tags *taglist, uint16_t tag)
{
	const struct IFD_tag *res = IFD_tag_get(taglist, tag);

	if (res == NULL)
		return "Unknown";
	else
		return res->name;
}

#include "gp_exif_gps.h"
#include "gp_exif.h"

static int get_buf(gp_io *io, off_t offset, char *buf, size_t len)
{
	off_t off;

	off = gp_io_tell(io);

	if (gp_io_seek(io, offset, GP_SEEK_SET) == -1) {
		GP_WARN("Failed to seek to data");
		return 1;
	}

	if (gp_io_fill(io, buf, len)) {
		GP_WARN("Failed to read data");
		return 1;
	}

	if (gp_io_seek(io, off, GP_SEEK_SET) == -1) {
		GP_WARN("Failed to seek back");
		return 1;
	}

	return 0;
}

static int load_string(gp_io *io, gp_storage *storage, gp_data_node *node,
                       const char *id, uint32_t num_comp, uint32_t *val)
{
	size_t max_len = GP_MIN(num_comp, 1024u);
	char buf[max_len];

	/* Short strings are stored in the value directly */
	if (num_comp <= 4) {
		memcpy(buf, val, num_comp);
		buf[num_comp > 0 ? num_comp - 1 : num_comp] = 0;
		goto add;
	}

	/* Longer are stored at offset starting from II or MM */
	if (get_buf(io, *val + 6, buf, max_len))
		return 1;

	buf[max_len - 1] = '\0';

add:
	GP_DEBUG(2, "ASCII String value = '%s'", buf);

	return gp_storage_add_string(storage, node, id, buf) != NULL;
}

static int load_rat(gp_io *io, gp_storage *storage, gp_data_node *node,
		    const char *id, uint32_t num_comp, uint32_t val)
{
	size_t max_comps = GP_MIN(num_comp, 32u);
	uint32_t buf[2 * max_comps];

	if (get_buf(io, val + 6, (void*)buf, num_comp * 8))
		return 1;

	//TODO: Data Storage needs array
	return gp_storage_add_rational(storage, node, id, buf[0], buf[1]) != NULL;
}

static int load_tag(gp_io *io, gp_storage *storage,
                    gp_data_node* node, const struct IFD_tags *taglist,
		    int endian, uint16_t tag, uint16_t format,
                    uint32_t num_comp, uint32_t val)
{
	const struct IFD_tag *res = IFD_tag_get(taglist, tag);

	if (res == NULL) {
		GP_TODO("Skipping unknown IFD tag 0x%02x %s cnt %u in %s block",
		        tag, IFD_format_name(format), num_comp, taglist->id);
		return 0;
	}

	if (res->format != format) {
		GP_WARN("Unexpected tag '%s' format '%s' (0x%02x) "
		        "expected '%s' in %s block", res->name,
		        IFD_format_name(format), format,
		        IFD_format_name(res->format), taglist->id);
		//TODO: Load and convert!
	}

	if ((res->num_components != 0) &&
	    (res->num_components != num_comp)) {
		GP_WARN("Unexpected '%s' num_components %u expected %u in %s block",
		        res->name, num_comp, res->num_components, taglist->id);
	}

	switch (format) {
	case IFD_ASCII_STRING:
		if (load_string(io, storage, node, res->name, num_comp, &val))
			return 1;
	break;
	case IFD_SIGNED_LONG:
	case IFD_UNSIGNED_LONG:
	case IFD_SIGNED_SHORT:
	case IFD_UNSIGNED_SHORT:
		if (num_comp == 1)
			gp_storage_add_int(storage, node, res->name, val);
		else
			goto unused;
	break;

	case IFD_UNSIGNED_RATIONAL:
	case IFD_SIGNED_RATIONAL:
		if (load_rat(io, storage, node, res->name, num_comp, val))
			return 1;
	break;
	case IFD_UNDEFINED:
		switch (res->tag) {
		case IFD_EXIF_VERSION:
		case IFD_FLASH_PIX_VERSION:
		case IFD_MAKER_NOTE:
		case IFD_INTEROP_VERSION:
			if (load_string(io, storage, node, res->name, num_comp, &val))
				return 1;
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

	return 0;
}

struct IFD_subrecord {
	uint16_t tag;
	uint32_t offset;
};

static int load_IFD(gp_io *io, gp_storage *storage, gp_data_node *node,
                    const struct IFD_tags *taglist, uint32_t IFD_offset,
                    int endian)
{
	uint16_t IFD_entries_count;
	uint16_t i2 = endian == 'I' ? GP_IO_L2 : GP_IO_B2;
	unsigned int i;

	uint16_t IFD_header[] = {
		GP_IO_IGN | IFD_offset,
		i2,
		GP_IO_END,
	};

	uint16_t IFD_record_LE[] = {
		GP_IO_L2, /* Tag                  */
		GP_IO_L2, /* Format               */
		GP_IO_L4, /* Number of components */
		GP_IO_L4, /* Value                */
		GP_IO_END,
	};

	uint16_t IFD_record_BE[] = {
		GP_IO_B2, /* Tag                  */
		GP_IO_B2, /* Format               */
		GP_IO_B4, /* Number of components */
		GP_IO_B4, /* Value                */
		GP_IO_END,
	};

	uint16_t *IFD_rec_head = endian == 'I' ? IFD_record_LE : IFD_record_BE;

	if (gp_io_readf(io, IFD_header, &IFD_entries_count) != 2) {
		GP_DEBUG(1, "Failed to read IFD entries count");
		return 1;
	}

	GP_DEBUG(2, "-- IFD Offset 0x%08x Entries %04u --",
	            IFD_offset, IFD_entries_count);

	struct IFD_subrecord subrecs[4];
	unsigned int subrec_cnt = 0;

	for (i = 0; i < IFD_entries_count; i++) {
		uint16_t tag, format;
		uint32_t num_comp, val;

		if (gp_io_readf(io, IFD_rec_head, &tag, &format, &num_comp, &val) != 4) {
			GP_DEBUG(1, "Failed to read IFD record");
			return 1;
		}

		GP_DEBUG(3, "IFD Entry tag 0x%04x format (0x%04x) components 0x%08x val 0x%08x",
		         tag, format, num_comp, val);

		GP_DEBUG(3, "IFD Entry tag '%s' format '%s'",
			 IFD_tag_name(taglist, tag), IFD_format_name(format));

		switch (tag) {
		case IFD_EXIF_OFFSET:
		case IFD_GPS_OFFSET:
		case IFD_INTEROPERABILITY_OFFSET:
			if (subrec_cnt >= GP_ARRAY_SIZE(subrecs)) {
				GP_WARN("Too much subrecords, skipping one");
				continue;
			}
			subrecs[subrec_cnt].tag = tag;
			subrecs[subrec_cnt].offset = val;
			subrec_cnt++;
		break;
		default:
			load_tag(io, storage, node, taglist, endian, tag, format, num_comp, val);
		break;
		}
	}

	for (i = 0; i < subrec_cnt; i++) {
		off_t cur_off = gp_io_tell(io);
		const struct IFD_tags *tags;
		gp_data_node *new_node;

		GP_DEBUG(3, "-- Loading sub IFD %s --",
		         IFD_tag_name(taglist, subrecs[i].tag));

		switch (subrecs[i].tag) {
		case IFD_EXIF_OFFSET:
			tags = taglist;
			new_node = node;
		break;
		case IFD_GPS_OFFSET:
			tags = &IFD_GPS_tags;
			new_node = gp_storage_add_dict(storage, node, "GPS");
		break;
		case IFD_INTEROPERABILITY_OFFSET:
			tags = taglist;
			new_node = gp_storage_add_dict(storage, node, "Interoperability");
		break;
		default:
			GP_BUG("Invalid tag");
			return 1;
		}

		/* Offset is counted from the II or MM in the Exif header */
		if (subrecs[i].offset + 6 < cur_off)
			GP_DEBUG(1, "Negative offset!");

		load_IFD(io, storage, new_node, tags, subrecs[i].offset + 6 - cur_off, endian);
	}

	return 0;
}

int gp_read_exif(gp_io *io, gp_storage *storage)
{
	char b1, b2;
	uint32_t IFD_offset;

	uint16_t exif_header[] = {
		'E', 'x', 'i', 'f', 0, 0, /* EXIF signature */
		GP_IO_BYTE, GP_IO_BYTE,   /* Endianity markers II or MM */
		GP_IO_END,
	};

	if (gp_io_readf(io, exif_header, &b1, &b2, &IFD_offset) != 8) {
		GP_DEBUG(1, "Failed to read Exif header");
		return 1;
	}

	if (b1 != b2 || (b1 != 'I' && b1 != 'M')) {
		GP_WARN("Expected II or MM got %x%x, corrupt header?", b1, b2);
		errno = EINVAL;
		return 1;
	}

	GP_DEBUG(2, "TIFF header endianity is '%c%c'", b1, b1);

	uint16_t tiff_header_LE[] = {
		0x2a, 0x00,               /* TIFF tag */
		GP_IO_L4,                 /* IFD offset */
		GP_IO_END,
	};

	uint16_t tiff_header_BE[] = {
		0x00, 0x2a,               /* TIFF tag */
		GP_IO_B4,                 /* IFD offset */
		GP_IO_END,
	};

	uint16_t *tiff_header = b1 == 'I' ? tiff_header_LE : tiff_header_BE;

	if (gp_io_readf(io, tiff_header,  &IFD_offset) != 3) {
		GP_DEBUG(1, "Failed to read TIFF header");
		return 1;
	}

	GP_DEBUG(2, "IFD offset is 0x%08x", IFD_offset);

	if (IFD_offset < 8) {
		GP_WARN("Invalid (negative) IFD offset");
		errno = EINVAL;
		return 1;
	}

	gp_data_node *exif_root = gp_storage_add_dict(storage, NULL, "Exif");

	/* The offset starts from the II or MM */
	return load_IFD(io, storage, exif_root, &IFD_EXIF_tags, IFD_offset - 8, b1);
}
