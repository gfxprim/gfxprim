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

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>

#include "tst_preload.h"
#include "tst_test.h"
#include "tst_job.h"

/*
 * Once we child forks to do a job, this points to its job structure.
 */
static struct tst_job *my_job = NULL;

static void start_test(struct tst_job *job)
{
	(void)job;
//	fprintf(stderr, "Starting test \e[1;37m%s\e[0m\n", job->test->name);
}

void tst_diff_timespec(int *sec, int *nsec, struct timespec *start,
                       struct timespec *stop)
{
	if (stop->tv_nsec < start->tv_nsec) {
		*sec  = stop->tv_sec - start->tv_sec - 1;
		*nsec = stop->tv_nsec + 1000000000 - start->tv_nsec;
	} else {
		*sec  = stop->tv_sec  - start->tv_sec;
		*nsec = stop->tv_nsec - start->tv_nsec;
	}
}

#define NAME_PADD 24

static void stop_test(struct tst_job *job)
{
	const char *name = job->test->name;
	int sec, nsec;
	const char *result = "";

	tst_diff_timespec(&sec, &nsec, &job->start_time, &job->stop_time);

	switch (job->result) {
	case TST_SUCCESS:
		result = "[    \e[1;32mSUCCESS\e[0m     ]";
	break;
	case TST_SKIPPED:
		result = "[    \e[1;30mSKIPPED\e[0m     ]";
	break;
	case TST_INTERR:
		result = "[ \e[1;31mINTERNAL ERROR\e[0m ]";
	break;
	case TST_SIGSEGV:
		result = "[    \e[1;31mSEGFAULT\e[0m    ]";
	break;
	case TST_TIMEOUT:
		result = "[    \e[1;35mTIMEOUT\e[0m     ]";
	break;
	case TST_ABORTED:
		result = "[    \e[1;31mABORTED\e[0m     ]";
	break;
	case TST_MEMLEAK:
		result = "[    \e[1;33mMEMLEAK\e[0m     ]";
	break;
	case TST_FAILED:
		result = "[    \e[1;31mFAILURE\e[0m     ]";
	break;
	case TST_MAX:
	break;
	}
		
	fprintf(stderr, "\e[1;37m%s\e[0m", name);
	
	int i;

	for (i = strlen(name); i < NAME_PADD; i++)
		fprintf(stderr, " ");

	fprintf(stderr, "  finished (Time %i.%03is, CPU %i.%03is)  %s\n",
	                sec, nsec/1000000,
			(int)job->cpu_time.tv_sec,
			(int)job->cpu_time.tv_nsec/1000000,
			result);

	if (job->result == TST_MEMLEAK)
		tst_malloc_print(&job->malloc_stats);

	/* Now print test message store */
	tst_msg_print(&job->store);
	
	fprintf(stderr, "------------------------------------------------------"
                        "------------------------- \n");
}

/*
 * Removes recursively temporary directory.
 */
static void remove_tmpdir(const char *path)
{
	/*
	 * Assert that we are working in /tmp/
	 */
	if (!strncmp("/tmp/", path, sizeof("/tmp/"))) {
		tst_warn("Path '%s' doesn't start with /tmp/, "
		         "omitting cleanup", path);
		return;
	}

	//TODO: Cleaner solution?
	char buf[256];
	int ret;

	snprintf(buf, sizeof(buf), "rm -rf '%s'", path);
	ret = system(buf);

	if (ret)
		tst_warn("Failed to clean temp dir.");	
}

/*
 * Create temp directory and cd into it, copy resources if needed
 */
static void prepare_tmpdir(const char *name, const char *res_path,
                          char *template, size_t size)
{
	char tmp[256];
	int ret;

	/* Fix any funny characters in the test name */
	snprintf(tmp, sizeof(tmp), "%s", name);

	char *s = tmp;

	while (*s != '\0') {
		if (!isalnum(*s))
			*s = '_';
		s++;
	}

	/* Create template from test name */
	snprintf(template, size, "/tmp/ctest_%s_XXXXXX", tmp);

	if (mkdtemp(template) == NULL) {
		tst_warn("mkdtemp(%s) failed: %s", template, strerror(errno));
		exit(TST_INTERR);
	}

	/* Copy resources if needed */
	if (res_path != NULL) {
		snprintf(tmp, sizeof(tmp), "cp -r '%s' '%s'",
		         res_path, template);

		ret = system(tmp);

		if (ret) {
			tst_warn("failed to copy resource '%s'", res_path);
			exit(TST_INTERR);
		}
	}


	if (chdir(template)) {
		tst_warn("chdir(%s) failed: %s", template, strerror(errno));
		exit(TST_INTERR);
	}
}

/*
 * Writes timespec into pipe
 */
static void write_timespec(struct tst_job *job, char type,
                           struct timespec *time)
{
	char buf[1 + sizeof(time_t) + sizeof(long)];
	char *ptr = buf;

	*(ptr++) = type;

	*((time_t*)ptr) = time->tv_sec;
	ptr += sizeof(time_t);
	*((long*)ptr) = time->tv_nsec;

	if (write(job->pipefd, buf, sizeof(buf)) != sizeof(buf))
		tst_warn("write(timespec) failed: %s", strerror(errno));
}

/*
 * Reads timespec from pipe
 */
static void read_timespec(struct tst_job *job, struct timespec *time)
{
	char buf[sizeof(time_t) + sizeof(long)];
	char *ptr = buf;

	if (read(job->pipefd, buf, sizeof(buf)) != sizeof(buf))
		tst_warn("read(timespec) failed: %s", strerror(errno));

	time->tv_sec = *((time_t*)ptr);
	ptr += sizeof(time_t);
	time->tv_nsec = *(long*)(ptr);
}

static void child_write(struct tst_job *job, char ch, void *ptr, ssize_t size)
{
	if (write(job->pipefd, &ch, 1) != 1)
		tst_warn("child write() failed: %s", strerror(errno));

	if (ptr != NULL) {
		if (write(job->pipefd, ptr, size) != size)
			tst_warn("child write() failed: %s", strerror(errno));
	}
}

int tst_report(int level, const char *fmt, ...)
{
	va_list va;
	int ret;
	char buf[258];

	va_start(va, fmt);
	ret = vsnprintf(buf+3, sizeof(buf) - 3, fmt, va);
	va_end(va);
	
	ssize_t size = ret > 255 ? 255 : ret + 1;

	buf[0] = 'm';
	buf[1] = level;
	((unsigned char*)buf)[2] = size; 

	if (my_job != NULL)
		if (write(my_job->pipefd, buf, size + 3) != size + 3)
			tst_warn("Failed to write msg to pipe.");

	return ret;
}

void tst_job_run(struct tst_job *job)
{
	int ret;
	char template[256];
	int pipefd[2];
	
	/* Write down starting time of the test */
	clock_gettime(CLOCK_MONOTONIC, &job->start_time);

	/* Prepare the test message store */
	tst_msg_init(&job->store);

	/* marks test as started */
	start_test(job);

	if (pipe(pipefd)) {
		tst_warn("pipefd() failed: %s", strerror(errno));
		job->running = 0;
		job->result = TST_INTERR;
		return;
	}

	job->pid = fork();

	switch (job->pid) {
	case -1:
		tst_warn("fork() failed: %s", strerror(errno));
		job->running = 0;
		job->result = TST_INTERR;
		return;
	case 0:
		close(pipefd[0]);
		job->pipefd = pipefd[1];
		my_job = job;
	break;
	default:
		close(pipefd[1]);
		job->pipefd = pipefd[0];
		job->running = 1;
		return;
	}

	/* Redirect stderr/stdout TODO: catch its output */
	if (freopen("/dev/null", "w", stderr))
		tst_warn("freopen(stderr) failed: %s", strerror(errno));

	if (freopen("/dev/null", "w", stdout))
		tst_warn("freopen(stderr) failed: %s", strerror(errno));

	/* Create directory in /tmp/ and chdir into it. */
	if (job->test->flags & TST_TMPDIR)
		prepare_tmpdir(job->test->name, job->test->res_path,
		               template, sizeof(template));

	/* 
	 * If timeout is specified, setup alarm.
	 *
	 * If alarm fires the test will be killed by SIGALRM.
	 */
	if (job->test->timeout)
		alarm(job->test->timeout);

	/* Send process cpu time to parent */
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &job->cpu_time);
	write_timespec(job, 'c', &job->cpu_time);

	if (job->test->flags & TST_CHECK_MALLOC)
		tst_malloc_check_start();

	/* Run test */
	ret = job->test->tst_fn();

	if (job->test->flags & TST_CHECK_MALLOC) {
		tst_malloc_check_stop();
		tst_malloc_check_report(&job->malloc_stats);

		child_write(job, 's', &job->malloc_stats,
                            sizeof(job->malloc_stats));

		if (job->malloc_stats.lost_chunks != 0 && ret == TST_SUCCESS)
			ret = TST_MEMLEAK;
	}

	/* Send process cpu time to parent */
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &job->cpu_time);
	write_timespec(job, 'C', &job->cpu_time);

	/* Cleanup temporary dir recursively */
	if (job->test->flags & TST_TMPDIR)
		remove_tmpdir(template);

	/* Send the parent we are done */
	child_write(job, 'x', NULL, 0);

	close(job->pipefd);

	exit(ret);
}

static void parent_read_msg(struct tst_job *job)
{
	unsigned char header[2];

	if (read(job->pipefd, header, sizeof(header)) != sizeof(header))
		tst_warn("parent: read(message header) failed: %s",
		         strerror(errno));

	char buf[header[1]];

	if (read(job->pipefd, buf, sizeof(buf)) != (ssize_t)sizeof(buf))
		tst_warn("parent: read(message) failed: %s", strerror(errno));

	/* null-terminated the string, to be extra sure */
	buf[header[1] - 1] = '\0';

	tst_msg_append(&job->store, header[0], buf);
}

static void parent_read(struct tst_job *job, void *ptr, ssize_t size)
{
	if (read(job->pipefd, ptr, size) != size)
		tst_warn("parent: read(): %s", strerror(errno));
}

void tst_job_wait(struct tst_job *job)
{
	int status, ret;
	char ch;

	if (!job->running)
		tst_warn("Job %s (pid %i) not in running state",
		         job->test->name, job->pid);

	while (job->running) {
		ret = read(job->pipefd, &ch, 1);

		if (ret < 0) {
			tst_warn("job_wait: read() failed: %s",
			         strerror(errno));
			job->running = 0;
			continue;
		}

		/* Child exited => read returns end of file */
		if (ret == 0) {
			job->running = 0;
			continue;
		}

		switch (ch) {
		/* test exited normally */
		case 'x':
			job->running = 0;
		break;
		/* cpu consumed time */
		case 'c':
			read_timespec(job, &job->cpu_time);
		break;
		case 'C':
			read_timespec(job, &job->cpu_time);
		break;
		/* test message as generated by tst_report() */
		case 'm':
			parent_read_msg(job);
		break;
		/* malloc stats */
		case 's':
			parent_read(job, &job->malloc_stats,
			            sizeof(job->malloc_stats));
		break;
		default:
			//TODO: internal error
		break;
		}
	}

	/* collect the test return status */
	waitpid(job->pid, &status, 0);

	close(job->pipefd);
	job->pipefd = -1;
	job->pid = -1;

	if (WIFEXITED(status)) {
		job->result = WEXITSTATUS(status);
	} else {
		switch (WTERMSIG(status)) {
		case SIGSEGV:
			job->result = TST_SIGSEGV;
		break;
		case SIGALRM:
			job->result = TST_TIMEOUT;
		break;
		/* 
		 * abort() called most likely double free or malloc data
		 * corruption
		 */
		case SIGABRT:
			job->result = TST_ABORTED;
		break;
		default:
			printf("%i\n", WTERMSIG(status));
			job->result = TST_INTERR;
		}
	}
	
	/* Write down stop time  */
	clock_gettime(CLOCK_MONOTONIC, &job->stop_time);

	stop_test(job);
}
