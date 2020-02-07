@ include header.t
@ include exif.t
/*
 * Exif parser tables.
 *
 * Copyright (C) 2009-2015 Cyril Hrubis <metan@ucw.cz>
 */
enum IFD_EXIF_tags {
@ for i in exif:
@     if len(i) > 4:
	/* {{ i[4] }} */
@     end
	{{ exif_tag_name(i) }} = {{ "0x%04x" % i[1] }};
@     if len(i) > 4:

@     end
@ end
};

/* Sorted by tag id */
static const struct IFD_tag IFD_EXIF_taglist[] = {
@ for i in exif:
@     if len(i) > 4:
	/* {{ i[4] }} */
@     end
	{ {{ exif_tag_name(i) }}, "{{ i[0] }}", {{ exif_tag_type(i) }}, {{ i[3] }}},
@     if len(i) > 4:

@     end
@ end
	{IFD_OFFSET_SCHEMA, "Offset Schema", IFD_SIGNED_LONG, 1},
};

static const struct IFD_tags IFD_EXIF_tags = {
	.tags = IFD_EXIF_taglist,
	.tag_cnt = GP_ARRAY_SIZE(IFD_EXIF_taglist),
	.id = "Exif",
};

