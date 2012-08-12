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

#include "tst_test.h"
#include "tst_job.h"
#include "tst_msg.h"
#include "tst_log.h"

static const char *ret_to_bg_color(enum tst_ret ret)
{
	switch (ret) {
	case TST_SUCCESS:
		return "#008000";
	case TST_INTERR:
		return "#800000";
	case TST_SIGSEGV:
		return "#e00000";
	case TST_TIMEOUT:
		return "#800080";
	case TST_MEMLEAK:
		return "#a0a000";
	case TST_FAILED:
		return "#e00000";
	case TST_MAX:
	break;
	}
		
	return "#000000";
}

static const char *ret_to_str(enum tst_ret ret)
{
	switch (ret) {
	case TST_SUCCESS:
		return "Success";
	case TST_INTERR:
		return "Internal Error";
	case TST_SIGSEGV:
		return "Segmentation Fault";
	case TST_TIMEOUT:
		return "Timeout";
	case TST_MEMLEAK:
		return "Memory Leak";
	case TST_FAILED:
		return "Failed";
	case TST_MAX:
	break;
	}

	return "Unknown";
}

static int append_html(struct tst_job *job, FILE *f)
{
	const char *padd = "   ";

	fprintf(f, "%s<tr>\n", padd);
	fprintf(f, "%s <td colspan=2 bgcolor=\"#ccccee\">%s</td>\n", padd, job->test->name);
	fprintf(f, "%s <td bgcolor=\"%s\"><font color=\"white\">%s</td>\n", padd,
	        ret_to_bg_color(job->result), ret_to_str(job->result));

	struct tst_msg *msg;

	for (msg = job->store.first; msg != NULL; msg = msg->next) {
		fprintf(f, "%s<tr>", padd);
		fprintf(f, "%s <td colspan=\"3\" bgcolor=\"#eeeeee\">", padd);
		fprintf(f, "%s  &nbsp;&nbsp;%s", padd, msg->msg);
		fprintf(f, "%s </td>", padd);
		fprintf(f, "%s</tr>", padd);
	}

	fprintf(f, "%s</tr>\n", padd);
	
	return 0;
}

int tst_log_append(struct tst_job *job, FILE *f, enum tst_log_fmt format)
{
	switch (format) {
	case TST_LOG_HTML:
		return append_html(job, f);
	break;
	default:
		return 1;
	}

	return 1;
}

FILE *open_html(const struct tst_suite *suite, const char *path)
{
	FILE *f;

	f = fopen(path, "w");

	if (f == NULL)
		return NULL;

	fprintf(f, "<html>\n <head>\n </head>\n <body>\n  <table>\n");

	fprintf(f, "   <tr><td colspan=\"3\" bgcolor=\"#bbbbff\"><b>%s"
	           "</b></td></tr>\n", suite->suite_name);

	return f;
}

FILE *tst_log_open(const struct tst_suite *suite, const char *path,
                   enum tst_log_fmt format)
{
	switch (format) {
	case TST_LOG_HTML:
		return open_html(suite, path);
	break;
	default:
		return NULL;
	}

	return NULL;
}

static int close_html(FILE *f)
{
	fprintf(f, "  </table>\n </body>\n</html>\n");
	fclose(f);
	return 0;
}

int tst_log_close(FILE *f, enum tst_log_fmt format)
{
	switch (format) {
	case TST_LOG_HTML:
		return close_html(f);
	break;
	default:
		return 1;
	}

	return 1;
}
