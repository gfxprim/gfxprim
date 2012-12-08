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
#include "tst_timespec.h"
#include "tst_log.h"

static const char *ret_to_str(enum tst_ret ret)
{
	switch (ret) {
	case TST_SUCCESS:
		return "Success";
	case TST_SKIPPED:
		return "Skipped";
	case TST_UNTESTED:
		return "Untested";
	case TST_INTERR:
		return "Internal Error";
	case TST_SIGSEGV:
		return "Segmentation Fault";
	case TST_TIMEOUT:
		return "Timeout";
	case TST_ABORTED:
		return "Aborted";
	case TST_FPE:
		return "FP Exception";
	case TST_MEMLEAK:
		return "Memory Leak";
	case TST_FAILED:
		return "Failed";
	case TST_MAX:
	break;
	}

	return "Unknown";
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

static void append_benchmark_json(struct tst_job *job, FILE *f)
{
	fprintf(f, "\t\t\t\"Benchmark\": {\n");

	fprintf(f, "\t\t\t\t\"Time Mean\": %i.%09i,\n",
	        (int)job->bench_mean.tv_sec,
	        (int)job->bench_mean.tv_nsec);
	
	fprintf(f, "\t\t\t\t\"Time Variance\": %i.%09i,\n",
	        (int)job->bench_var.tv_sec,
	        (int)job->bench_var.tv_nsec);

	fprintf(f, "\t\t\t\t\"Iterations\": %i\n", job->bench_iter);

	fprintf(f, "\t\t\t},\n");
}

static int hack_json_start = 0;

int tst_log_append(struct tst_job *job, FILE *f)
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

	/* If benchmark data were created */
	if (job->bench_iter)
		append_benchmark_json(job, f);

	/* Time statistics */
	int sec, nsec;

	timespec_diff(&sec, &nsec, &job->start_time, &job->stop_time);
	
	fprintf(f, "\t\t\t\"CPU Time\": %i.%09i,\n",
	        (int)job->cpu_time.tv_sec, (int)job->cpu_time.tv_nsec);

	fprintf(f, "\t\t\t\"Run Time\": %i.%09i\n", sec, nsec);
	
	fprintf(f, "\t\t}");

	return 0;
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
	FILE *cmd = popen("lscpu 2> /dev/null", "r");

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

FILE *tst_log_open(const struct tst_suite *suite, const char *path)
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

int tst_log_close(FILE *f)
{
	fprintf(f, "\n\t]\n}\n");
	return fclose(f);
}
