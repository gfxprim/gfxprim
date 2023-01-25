// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2022-2023 Cyril Hrubis <metan@ucw.cz>
 */

#include "markup.h"

static int markup_parse_html(struct markup_tcase *tcase)
{
	return markup_parse(GP_MARKUP_HTML, tcase);
}

static struct markup_tcase html_empty_01 = {
	.markup_str = " ",
	.markup_glyphs = {
		{}
	}
};

static struct markup_tcase html_empty_02 = {
	.markup_str = " \t\n\n",
	.markup_glyphs = {
		{}
	}
};

static struct markup_tcase html_unclosed_tag = {
	.markup_str = "hello <b>world",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{.glyph = ' '},
		{.glyph = 'w', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'o', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'r', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'l', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'd', .fmt = GP_MARKUP_BOLD},
		{}
	}
};

static struct markup_tcase html_whitespaces = {
	.markup_str = " hello \n\t\r world ",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{.glyph = ' '},
		{.glyph = 'w'}, {.glyph = 'o'}, {.glyph = 'r'}, {.glyph = 'l'}, {.glyph = 'd'},
		{}
	}
};

static struct markup_tcase html_unknown_tags = {
	.markup_str = "hello <unknown bar=\"val\"> world",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{.glyph = ' '},
		{.glyph = 'w'}, {.glyph = 'o'}, {.glyph = 'r'}, {.glyph = 'l'}, {.glyph = 'd'},
		{}
	}
};

static struct markup_tcase html_escape = {
	.markup_str = "&amp;&lt;&gt;&nbsp;&copy;&reg;&deg;&plusmn;&micro;",
	.markup_glyphs = {
		{.glyph = '&'}, {.glyph = '<'}, {.glyph = '>'},
		{.glyph = 0xa0}, {.glyph = 0xa9}, {.glyph = 0xae},
		{.glyph = 0xb0}, {.glyph = 0xb1}, {.glyph = 0xb4},
		{}
	}
};

static struct markup_tcase html_bold = {
	.markup_str = "hello <b>world</b>",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{.glyph = ' '},
		{.glyph = 'w', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'o', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'r', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'l', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'd', .fmt = GP_MARKUP_BOLD},
		{}
	}
};

static struct markup_tcase html_br = {
	.markup_str = "hello<br><BR><br /><BR />world",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{.glyph = '\n'}, {.glyph = '\n'}, {.glyph = '\n'}, {.glyph = '\n'},
		{.glyph = 'w'}, {.glyph = 'o'}, {.glyph = 'r'}, {.glyph = 'l'}, {.glyph = 'd'},
		{}
	}
};

static struct markup_tcase html_hr = {
	.markup_str = "hello<hr><HR><hr /><HR />world",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{.glyph = '\n', .fmt = GP_MARKUP_STRIKE},
		{.glyph = '\n', .fmt = GP_MARKUP_STRIKE},
		{.glyph = '\n', .fmt = GP_MARKUP_STRIKE},
		{.glyph = '\n', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'w'}, {.glyph = 'o'}, {.glyph = 'r'}, {.glyph = 'l'}, {.glyph = 'd'},
		{}
	}
};

static struct markup_tcase html_s = {
	.markup_str = "<s>hello world</s>",
	.markup_glyphs = {
		{.glyph = 'h', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'e', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'l', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'l', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'o', .fmt = GP_MARKUP_STRIKE},
		{.glyph = ' ', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'w', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'o', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'r', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'l', .fmt = GP_MARKUP_STRIKE},
		{.glyph = 'd', .fmt = GP_MARKUP_STRIKE},
		{}
	}
};

static struct markup_tcase html_span_01 = {
	.markup_str = "<span font_weight=\"bold\">hello<span strikethrough=\"true\">cruel</span>world!</span>",
	.markup_glyphs = {
		{.glyph = 'h', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'e', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'l', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'l', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'o', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'c', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'r', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'u', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'e', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'l', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'w', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'o', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'r', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'l', .fmt = GP_MARKUP_BOLD},
		{.glyph = 'd', .fmt = GP_MARKUP_BOLD},
		{.glyph = '!', .fmt = GP_MARKUP_BOLD},
		{}
	}
};

static struct markup_tcase html_span_02 = {
	.markup_str = "<span strikethrough=\"true\"><b>hello</span>world!",
	.markup_glyphs = {
		{.glyph = 'h', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'e', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'l', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'l', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'o', .fmt = GP_MARKUP_BOLD | GP_MARKUP_STRIKE},
		{.glyph = 'w'}, {.glyph = 'o'}, {.glyph = 'r'},
		{.glyph = 'l'}, {.glyph = 'd'}, {.glyph = '!'},
		{}
	}
};

static struct markup_tcase html_sup_sub = {
	.markup_str = "<sup>hello<sub>cruel</sub>world</sup><sub>sub</sub>",
	.markup_glyphs = {
		{.glyph = 'h', .fmt = GP_MARKUP_SUP},
		{.glyph = 'e', .fmt = GP_MARKUP_SUP},
		{.glyph = 'l', .fmt = GP_MARKUP_SUP},
		{.glyph = 'l', .fmt = GP_MARKUP_SUP},
		{.glyph = 'o', .fmt = GP_MARKUP_SUP},
		{.glyph = 'c', .fmt = GP_MARKUP_SUP},
		{.glyph = 'r', .fmt = GP_MARKUP_SUP},
		{.glyph = 'u', .fmt = GP_MARKUP_SUP},
		{.glyph = 'e', .fmt = GP_MARKUP_SUP},
		{.glyph = 'l', .fmt = GP_MARKUP_SUP},
		{.glyph = 'w', .fmt = GP_MARKUP_SUP},
		{.glyph = 'o', .fmt = GP_MARKUP_SUP},
		{.glyph = 'r', .fmt = GP_MARKUP_SUP},
		{.glyph = 'l', .fmt = GP_MARKUP_SUP},
		{.glyph = 'd', .fmt = GP_MARKUP_SUP},
		{.glyph = 's', .fmt = GP_MARKUP_SUB},
		{.glyph = 'u', .fmt = GP_MARKUP_SUB},
		{.glyph = 'b', .fmt = GP_MARKUP_SUB},
		{}
	}
};

static struct markup_tcase html_tt = {
	.markup_str = "<tt>hello world</tt>",
	.markup_glyphs = {
		{.glyph = 'h', .fmt = GP_MARKUP_MONO},
		{.glyph = 'e', .fmt = GP_MARKUP_MONO},
		{.glyph = 'l', .fmt = GP_MARKUP_MONO},
		{.glyph = 'l', .fmt = GP_MARKUP_MONO},
		{.glyph = 'o', .fmt = GP_MARKUP_MONO},
		{.glyph = ' ', .fmt = GP_MARKUP_MONO},
		{.glyph = 'w', .fmt = GP_MARKUP_MONO},
		{.glyph = 'o', .fmt = GP_MARKUP_MONO},
		{.glyph = 'r', .fmt = GP_MARKUP_MONO},
		{.glyph = 'l', .fmt = GP_MARKUP_MONO},
		{.glyph = 'd', .fmt = GP_MARKUP_MONO},
		{}
	}
};

static struct markup_tcase html_u = {
	.markup_str = "<u>hello world</u>",
	.markup_glyphs = {
		{.glyph = 'h', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = 'e', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = 'l', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = 'l', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = 'o', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = ' ', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = 'w', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = 'o', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = 'r', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = 'l', .fmt = GP_MARKUP_UNDERLINE},
		{.glyph = 'd', .fmt = GP_MARKUP_UNDERLINE},
		{}
	}
};

static struct markup_tcase html_strip_tags = {
	.markup_str = "hel<script>foo</script>lo<style>bar</style><xml><data>bar</data></xml>",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{}
	}
};

static struct markup_tcase html_p = {
	.markup_str = "<p>hello</p><p>world</p>",
	.markup_glyphs = {
		{.glyph = 'h'}, {.glyph = 'e'}, {.glyph = 'l'}, {.glyph = 'l'}, {.glyph = 'o'},
		{.glyph = '\n'},
		{.glyph = 'w'}, {.glyph = 'o'}, {.glyph = 'r'}, {.glyph = 'l'}, {.glyph = 'd'},
		{.glyph = '\n'},
		{}
	}
};

const struct tst_suite tst_suite = {
	.suite_name = "markup parser testsuite",
	.tests = {
		{.name = "markup html empty 01",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_empty_01},

		{.name = "markup html empty 02",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_empty_02},

		{.name = "markup html unclosed tag",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_unclosed_tag},

		{.name = "markup html whitespaces",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_whitespaces},

		{.name = "markup unknown tags",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_unknown_tags},

		{.name = "markup html &xxx; escape",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_escape},

		{.name = "markup html <b>",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_bold},

		{.name = "markup html <br>",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_br},

		{.name = "markup html <hr>",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_hr},

		{.name = "markup html <s>",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_s},

		{.name = "markup html <span> 01",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_span_01},

		{.name = "markup html <span> 02",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_span_02},

		{.name = "markup html <sup> <sub>",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_sup_sub},

		{.name = "markup html <tt>",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_tt},

		{.name = "markup html <u>",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_u},

		{.name = "markup strip <script> <style> <xml>",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_strip_tags},

		{.name = "markup html <p>",
		 .tst_fn = markup_parse_html,
		 .flags = TST_CHECK_MALLOC,
		 .data = &html_p},

		{.name = NULL},
	}
};
