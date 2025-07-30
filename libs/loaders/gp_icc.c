// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2024 Cyril Hrubis <metan@ucw.cz>
 */

/*
 * Minimalistic ICC profile parser.
 */

#include <errno.h>
#include <string.h>

#include <core/gp_common.h>
#include <core/gp_debug.h>
#include <core/gp_byte_order.h>
#include <loaders/gp_icc.h>

struct icc_tag {
	char sig[5];
	uint32_t offset;
	uint32_t size;
};

#define GP_ICC_VER_MAJOR(val) (10 * (((val)>>28)&0xf) + (((val)>>24)&0xf))
#define GP_ICC_VER_MINOR(val) (((val)>>20)&0xf)
#define GP_ICC_VER_BUGFIX(val) (((val)>>16)&0xf)

#define ICC_SIG(a, b, c, d) \
	(((uint32_t)d)<<24 | \
	 ((uint32_t)c)<<16 | \
	 ((uint32_t)b)<<8  | \
	 ((uint32_t)a))

#define ICC_TAG_SIG(tag) \
	ICC_SIG((tag).sig[0], (tag).sig[1], (tag).sig[2], (tag).sig[3])

static char *read_icc_tag_str(gp_io *io, const struct icc_tag *tag)
{
	uint16_t str_tag_header[] = {
		GP_IO_ARRAY | 4, /* 'text' for ASCII string, 'mluc' for Unicode struct */
		0, 0, 0, 0, /* Reserved must be 0 */
		GP_IO_END
	};
	struct icc_tag text_tag = {};
	char *str;
	size_t text_size = GP_MAX(tag->size, 1u);

	if (gp_io_seek(io, tag->offset, GP_SEEK_SET) == (off_t)-1) {
		GP_WARN("Failed to seek to %s tag header", tag->sig);
		return NULL;
	}

	if (gp_io_readf(io, str_tag_header, &text_tag.sig) != 5) {
		GP_WARN("Failed to read %s text header", tag->sig);
		return NULL;
	}

	switch (ICC_TAG_SIG(text_tag)) {
	case ICC_SIG('t', 'e', 'x', 't'):
		GP_DEBUG(4, "Tag %s header signature %s", tag->sig, text_tag.sig);
		str = malloc(text_size);
		if (!str)
			return NULL;

		if (gp_io_read(io, str, tag->size) < 0) {
			GP_WARN("Failed to read %s tag text", tag->sig);
			free(str);
			return NULL;
		}
		str[text_size-1] = 0;

		return str;
	case ICC_SIG('m', 'l', 'u', 'c'):
		GP_TODO("mluc Unicode");
	break;
	default:
		GP_WARN("Invalid %s tag header %s", tag->sig, text_tag.sig);
		return NULL;
	}

	return NULL;
}

static char *read_icc_tag_desc(gp_io *io, const struct icc_tag *desc)
{
	uint16_t desc_header[] = {
		'd', 'e', 's', 'c',
		0, 0, 0, 0,
		GP_IO_B4, /* ASCII desc len */
		GP_IO_END
	};
	uint32_t ascii_len = 0;
	char *str;

	if (gp_io_seek(io, desc->offset, GP_SEEK_SET) == (off_t)-1) {
		GP_WARN("Failed to seek to desc tag");
		goto err0;
	}

	if (gp_io_readf(io, desc_header, &ascii_len) != 9) {
		GP_WARN("Failed to read desc tag ascii lenght");
		goto err0;
	}

	str = malloc(ascii_len);
	if (!str) {
		GP_WARN("Malloc failed :(");
		goto err0;
	}

	if (gp_io_read(io, str, ascii_len) != ascii_len) {
		GP_WARN("Failed to read ascii desc");
		goto err1;
	}

	return str;
err1:
	free(str);
err0:
	return NULL;
}

struct icc_header {
	uint32_t size;
	char type[5];
	uint32_t version;
	char class[5];
	char color_space[5];
	uint32_t conn_space;
	uint16_t c_year;
	uint16_t c_month;
	uint16_t c_day;
	uint16_t c_hour;
	uint16_t c_min;
	uint16_t c_sec;
	char primary_plat[5];
	uint32_t cmm_flags;
	uint32_t render_intent;
	uint32_t tag_cnt;
};

int gp_read_icc(gp_io *io, gp_storage *storage)
{
	uint16_t icc_header[] = {
		GP_IO_B4, /* size */
		GP_IO_ARRAY | 4, /* type */
		GP_IO_B4, /* version */
		GP_IO_ARRAY | 4, /* class */
		GP_IO_ARRAY | 4, /* color space */
		GP_IO_B4, /* profile connection space */
		/* Date time, year, month, day, hour, min, sec */
		GP_IO_B2, GP_IO_B2, GP_IO_B2,
		GP_IO_B2, GP_IO_B2, GP_IO_B2,
		'a', 'c', 's', 'p', /* signature */
		GP_IO_ARRAY | 4, /* primary platform */
		GP_IO_B4, /* cmm flags */
		GP_IO_IGN | 4, /* device manufacturer */
		GP_IO_IGN | 4, /* device model */
		GP_IO_IGN | 4, /* device attributes */
		GP_IO_B4, /* rendering intent */
		GP_IO_IGN | 12, /* connection space illuminant 4 bytes for each X Y Z */
		GP_IO_IGN | 4, /* profile creator */
		GP_IO_IGN | 4, /* profile ID */
		GP_IO_IGN | 44,
		GP_IO_B4, /* tag count */
		GP_IO_END,
	};

	struct icc_header h = {};
	ssize_t ret;
	uint32_t i;
	char *str;

	ret = gp_io_readf(io, icc_header, &h.size, &h.type, &h.version, &h.class,
	                  &h.color_space, &h.conn_space,
			  &h.c_year, &h.c_month, &h.c_day, &h.c_hour, &h.c_min, &h.c_sec,
			  &h.primary_plat, &h.cmm_flags, &h.render_intent, &h.tag_cnt);

	if (ret != 27) {
		GP_WARN("Failed to parse ICC header!");
		return 1;
	}

	GP_DEBUG(1, "ICC Profile version=%u.%u.%u created %4u:%02u:%02u %02u:%02u:%02u type=%s class=%s color_space=%s tag_count=%u",
		 (unsigned int)GP_ICC_VER_MAJOR(h.version),
		 (unsigned int)GP_ICC_VER_MINOR(h.version),
		 (unsigned int)GP_ICC_VER_BUGFIX(h.version),
		 h.c_year, h.c_month, h.c_day, h.c_hour, h.c_min, h.c_sec,
		 h.type, h.class, h.color_space, h.tag_cnt);

	gp_data_node *icc_root = gp_storage_add_dict(storage, NULL, "ICC Profile");

	//TODO: Add gp_storage_add_printf()
	char buf[64];

	snprintf(buf, sizeof(buf), "%u.%u.%u",
		 (unsigned int)GP_ICC_VER_MAJOR(h.version),
		 (unsigned int)GP_ICC_VER_MINOR(h.version),
		 (unsigned int)GP_ICC_VER_BUGFIX(h.version));

	gp_storage_add_string(storage, icc_root, "Version", buf);

	snprintf(buf, sizeof(buf), "%4u:%02u:%02u %02u:%02u:%02u",
		 h.c_year, h.c_month, h.c_day, h.c_hour, h.c_min, h.c_sec);

	gp_storage_add_string(storage, icc_root, "Created", buf);

	// TODO: assert correct values!
	gp_storage_add_string(storage, icc_root, "Color Space", h.color_space);
	gp_storage_add_string(storage, icc_root, "Class", h.class);
	gp_storage_add_string(storage, icc_root, "Type", h.type);
	gp_storage_add_string(storage, icc_root, "Primary Platform", h.primary_plat);

	const uint16_t icc_tag[] = {
		GP_IO_ARRAY | 4, /* Tag signature */
		GP_IO_B4, /* offset */
		GP_IO_B4, /* size */
		GP_IO_END
	};

	struct icc_tag rTRC = {}, gTRC = {}, bTRC = {}, desc = {}, cprt = {};

	for (i = 0; i < h.tag_cnt; i++) {
		struct icc_tag tag = {};

		ret = gp_io_readf(io, icc_tag, &tag.sig, &tag.offset, &tag.size);
		if (ret != 3) {
			GP_WARN("Failed to read ICC tag %s", strerror(errno));
			return 1;
		}

		switch (ICC_TAG_SIG(tag)) {
		case ICC_SIG('r', 'T', 'R', 'C'):
			rTRC = tag;
		break;
		case ICC_SIG('g', 'T', 'R', 'C'):
			gTRC = tag;
		break;
		case ICC_SIG('b', 'T', 'R', 'C'):
			bTRC = tag;
		break;
		case ICC_SIG('d', 'e', 's', 'c'):
			desc = tag;
		break;
		case ICC_SIG('c', 'p', 'r', 't'):
			cprt = tag;
		break;
		}

		GP_DEBUG(3, "tag='%s' off=%u size=%u", tag.sig, tag.offset, tag.size);
	}

	if (!rTRC.sig[0] || !gTRC.sig[0] || !bTRC.sig[0]) {
		GP_WARN("Profile does not contain RGB TRC curves!");
		return 1;
	}

	if (rTRC.sig[0]) {
		switch (rTRC.size) {
		case 0:
			GP_DEBUG(1, "Red channel is linear");
		break;
		case 1:
			GP_DEBUG(1, "Red channel gamma");
		break;
		default:
			GP_DEBUG(1, "Red channel curve with %u points", rTRC.size);
		break;
		}
	}

	if (cprt.sig[0]) {
		str = read_icc_tag_str(io, &cprt);
		if (str)
			gp_storage_add_string(storage, icc_root, "Copyright", str);
		free(str);
	}

	if (desc.sig[0]) {
		str = read_icc_tag_desc(io, &desc);
		if (str)
			gp_storage_add_string(storage, icc_root, "Description", str);
		free(str);
	}

	return 0;
}
