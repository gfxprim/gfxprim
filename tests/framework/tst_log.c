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
#include "tst_preload.h"
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
	case TST_ABORTED:
		return "#e00000";
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
	case TST_ABORTED:
		return "Aborted";
	case TST_MEMLEAK:
		return "Memory Leak";
	case TST_FAILED:
		return "Failed";
	case TST_MAX:
	break;
	}

	return "Unknown";
}

static void malloc_stats_html(struct tst_job *job, FILE *f, const char *padd)
{
	/* Create innter table */
	fprintf(f, "%s<tr>\n", padd);
	fprintf(f, "%s <td bgcolor=\"#ffffcc\" colspan=\"3\">\n", padd);
	fprintf(f, "%s  <center>\n", padd);
	fprintf(f, "%s   <table>\n", padd);

	/* Create header */
	fprintf(f, "%s    <tr>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <small>Total size</small>\n", padd);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <small>Total chunks</small>\n", padd);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <small>Lost size</small>\n", padd);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <small>Lost chunks</small>\n", padd);
	fprintf(f, "%s     </td>\n", padd);

	fprintf(f, "%s    </tr>\n", padd);

	/* Create data */
	fprintf(f, "%s    <tr>\n", padd);

	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <center><small>%zu</small></center>\n",
	           padd, job->malloc_stats.total_size);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <center><small>%u</small></center>\n",
	           padd, job->malloc_stats.total_chunks);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <center><small>%zu</small></center>\n",
	           padd, job->malloc_stats.lost_size);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <center><small>%u</small></center>\n",
	           padd, job->malloc_stats.lost_chunks);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s    </tr>\n", padd);
	
	fprintf(f, "%s   </table>\n", padd);
	fprintf(f, "%s  </center>\n", padd);
	fprintf(f, "%s </td>\n", padd);
	fprintf(f, "%s</tr>\n", padd);
}

static int append_html(struct tst_job *job, FILE *f)
{
	const char *padd = "   ";
	int sec, nsec;

	tst_diff_timespec(&sec, &nsec, &job->start_time, &job->stop_time);

	fprintf(f, "%s<tr>\n", padd);
	fprintf(f, "%s <td bgcolor=\"#ccccee\">%s&nbsp;</td>\n", padd, job->test->name);
	fprintf(f, "%s <td bgcolor=\"#ccccee\">\n", padd);
	fprintf(f, "%s  <small><font color=\"#222\">%i.%03is %i.%03is</font></small>",
	        padd, sec, nsec/1000000, (int)job->cpu_time.tv_sec, (int)job->cpu_time.tv_nsec/1000000);
	fprintf(f, "%s </td>\n", padd);
	fprintf(f, "%s <td bgcolor=\"%s\"><center><font color=\"white\">&nbsp;%s&nbsp;</td></center>\n", padd,
	        ret_to_bg_color(job->result), ret_to_str(job->result));

	struct tst_msg *msg;

	/* If calculated include malloc report */
	if (job->test->flags & TST_MALLOC_CHECK)
		malloc_stats_html(job, f, padd);

	for (msg = job->store.first; msg != NULL; msg = msg->next) {
		fprintf(f, "%s<tr>\n", padd);
		fprintf(f, "%s <td colspan=\"3\" bgcolor=\"#eeeeee\">\n", padd);
		fprintf(f, "%s  &nbsp;&nbsp;<small>%s</small>\n", padd, msg->msg);
		fprintf(f, "%s </td>\n", padd);
		fprintf(f, "%s</tr>\n", padd);
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

	fprintf(f, "<html>\n <head>\n </head>\n <body>\n  <table bgcolor=\"#99a\">\n");

	fprintf(f, "   <tr><td colspan=\"3\" bgcolor=\"#bbbbff\"><center><b>%s"
	           "</b></center></td></tr>\n", suite->suite_name);

	fprintf(f, "   <tr>\n");
	fprintf(f, "    <td bgcolor=\"#eee\"><center>Test Name</center></td>\n");
	fprintf(f, "    <td bgcolor=\"#eee\"><center>Time/CPU</center></td>\n");
	fprintf(f, "    <td bgcolor=\"#eee\"><center>Result</center></td>\n");
	fprintf(f, "   </tr>\n");

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
