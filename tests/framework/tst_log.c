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

#include <sys/utsname.h>
#include <stdio.h>

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
	case TST_SKIPPED:
		return "#888888";
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
	case TST_SKIPPED:
		return "Skipped";
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

static void bytes_human_readable(FILE *f, size_t bytes)
{
	if (bytes < 512) {
		fprintf(f, "%zuB", bytes);
		return;
	}

	if (bytes < 1024 * 512) {
		fprintf(f, "%.2fkB", (float)bytes / 1024);
		return;
	}

	if (bytes < 1024 * 1024 * 512) {
		fprintf(f, "%.2fMB", (float)bytes / 1024 / 1024);
		return;
	}

	fprintf(f, "%.2fGB", (float)bytes / 1024 / 1024 / 1024);
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
	fprintf(f, "%s      <small>Max size</small>\n", padd);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <small>Max chunks</small>\n", padd);
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
	fprintf(f, "%s      <center><small>", padd);
	bytes_human_readable(f, job->malloc_stats.total_size);
	fprintf(f, "</small></center>\n");
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <center><small>%u</small></center>\n",
	           padd, job->malloc_stats.total_chunks);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <center><small>", padd);
	bytes_human_readable(f, job->malloc_stats.max_size);
	fprintf(f, "</small></center>\n");
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <center><small>%u</small></center>\n",
	           padd, job->malloc_stats.max_chunks);
	fprintf(f, "%s     </td>\n", padd);
	
	fprintf(f, "%s     <td bgcolor=\"#ffffaa\">\n", padd);
	fprintf(f, "%s      <center><small>", padd);
	bytes_human_readable(f, job->malloc_stats.lost_size);
	fprintf(f, "</small></center>\n");
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
	static int hack_counter = 0;
	const char *bgcol;

	tst_diff_timespec(&sec, &nsec, &job->start_time, &job->stop_time);

	if (hack_counter)
		bgcol = "#ccccee";
	else
		bgcol = "#ddddee";

	fprintf(f, "%s<tr>\n", padd);
	fprintf(f, "%s <td bgcolor=\"%s\">%s&nbsp;</td>\n", padd, bgcol, job->test->name);
	fprintf(f, "%s <td bgcolor=\"%s\">\n", padd, bgcol);
	fprintf(f, "%s  <center><small><font color=\"#222\">%i.%03is %i.%03is</font></small></center>",
	        padd, sec, nsec/1000000, (int)job->cpu_time.tv_sec, (int)job->cpu_time.tv_nsec/1000000);
	fprintf(f, "%s </td>\n", padd);
	fprintf(f, "%s <td bgcolor=\"%s\"><center><font color=\"white\">&nbsp;%s&nbsp;</td></center>\n", padd,
	        ret_to_bg_color(job->result), ret_to_str(job->result));

	struct tst_msg *msg;

	/* If calculated include malloc report */
	if (job->test->flags & TST_CHECK_MALLOC)
		malloc_stats_html(job, f, padd);

	for (msg = job->store.first; msg != NULL; msg = msg->next) {
		fprintf(f, "%s<tr>\n", padd);
		fprintf(f, "%s <td colspan=\"3\" bgcolor=\"#eeeeee\">\n", padd);
		fprintf(f, "%s  &nbsp;&nbsp;<small>%s</small>\n", padd, msg->msg);
		fprintf(f, "%s </td>\n", padd);
		fprintf(f, "%s</tr>\n", padd);
	}

	fprintf(f, "%s</tr>\n", padd);

	hack_counter = !hack_counter;

	return 0;
}

static int append_msg_json(struct tst_job *job, FILE *f)
{
	struct tst_msg *msg;

	fprintf(f, "\t\t\t\"Test Reports\": [\n");

	for (msg = job->store.first; msg != NULL; msg = msg->next) {
		fprintf(f, "\t\t\t\t\"%s\"", msg->msg);

		if (msg->next != NULL)
			fprintf(f, ",\n");
	}

	fprintf(f, "\n\t\t\t],\n");

	return 0;
}

static int append_malloc_stats_json(struct tst_job *job, FILE *f)
{
	fprintf(f, "\t\t\t\"Malloc Stats\": {\n");
	fprintf(f, "\t\t\t\t\"Total Size\": %zi,\n",
	        job->malloc_stats.total_size);
	fprintf(f, "\t\t\t\t\"Total Chunks\": %u,\n",
	        job->malloc_stats.total_chunks);
	fprintf(f, "\t\t\t\t\"Max Size\": %zi,\n",
	        job->malloc_stats.max_size);
	fprintf(f, "\t\t\t\t\"Max Chunks\": %u,\n",
	        job->malloc_stats.max_chunks);
	fprintf(f, "\t\t\t\t\"Lost Size\": %zi,\n",
	        job->malloc_stats.lost_size);
	fprintf(f, "\t\t\t\t\"Lost Chunks\": %u\n",
	        job->malloc_stats.lost_chunks);
	fprintf(f, "\t\t\t},\n");

	return 0;
}

static int hack_json_start = 0;

static int append_json(struct tst_job *job, FILE *f)
{
	if (hack_json_start)
		hack_json_start = 0;
	else
		fprintf(f, ",\n");

	fprintf(f, "\t\t{\n");
	fprintf(f, "\t\t\t\"Test Name\": \"%s\",\n", job->test->name);
	fprintf(f, "\t\t\t\"Test Result\": \"%s\",\n", ret_to_str(job->result));

	/* Append any test reports */
	append_msg_json(job, f);
	
	/* If calculated include malloc report */
	if (job->test->flags & TST_CHECK_MALLOC)
		append_malloc_stats_json(job, f);
	
	/* Time statistics */
	int sec, nsec;

	tst_diff_timespec(&sec, &nsec, &job->start_time, &job->stop_time);
	
	fprintf(f, "\t\t\t\"CPU Time\": %i.%09i,\n",
	        (int)job->cpu_time.tv_sec, (int)job->cpu_time.tv_nsec);

	fprintf(f, "\t\t\t\"Run Time\": %i.%09i\n", sec, nsec);
	
	fprintf(f, "\t\t}");

	return 0;
}

int tst_log_append(struct tst_job *job, FILE *f, enum tst_log_fmt format)
{
	switch (format) {
	case TST_LOG_HTML:
		return append_html(job, f);
	break;
	case TST_LOG_JSON:
		return append_json(job, f);
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

static void write_system_info_json(FILE *f)
{
	struct utsname buf;

	uname(&buf);

	fprintf(f, "\t\"System Info\": {\n");
	
	fprintf(f, "\t\t\"OS\": \"%s\",\n", buf.sysname);
	fprintf(f, "\t\t\"Hostname\": \"%s\",\n", buf.nodename);
	fprintf(f, "\t\t\"Release\": \"%s\",\n", buf.release);

	/* CPU related info */
	fprintf(f, "\t\t\"CPU\": {");
	
	/* lscpu is part of reasonably new util-linux */
	FILE *cmd = popen("lscpu", "r");

	if (cmd != NULL) {
		char id[256], val[1024];
		char *del = "\n";

		while (fscanf(cmd, "%[^:]%*c %[^\n]\n", id, val) == 2) {
			fprintf(f, "%s\t\t\t\"%s\": \"%s\"", del, id, val);
			del = ",\n";
		}
	
		fclose(cmd);
		fprintf(f, "\n");
	}
	
	fprintf(f, "\t\t}\n");

	fprintf(f, "\t},\n");
}

FILE *open_json(const struct tst_suite *suite, const char *path)
{
	FILE *f;

	f = fopen(path, "w");

	if (f == NULL)
		return NULL;

	fprintf(f, "{\n");
	fprintf(f, "\t\"Suite Name\": \"%s\",\n", suite->suite_name);
	write_system_info_json(f);
	fprintf(f, "\t\"Test Results\": [\n");

	hack_json_start = 1;

	return f;
}

FILE *tst_log_open(const struct tst_suite *suite, const char *path,
                   enum tst_log_fmt format)
{
	switch (format) {
	case TST_LOG_HTML:
		return open_html(suite, path);
	break;
	case TST_LOG_JSON:
		return open_json(suite, path);
	break;
	default:
		return NULL;
	}

	return NULL;
}

static int close_html(FILE *f)
{
	fprintf(f, "  </table>\n </body>\n</html>\n");
	return fclose(f);
}

static int close_json(FILE *f)
{
	fprintf(f, "\n\t]\n}\n");
	return fclose(f);
}

int tst_log_close(FILE *f, enum tst_log_fmt format)
{
	switch (format) {
	case TST_LOG_HTML:
		return close_html(f);
	break;
	case TST_LOG_JSON:
		return close_json(f);
	break;
	default:
		return 1;
	}

	return 1;
}
