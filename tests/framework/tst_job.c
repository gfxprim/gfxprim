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

#include "tst_preload.h"
#include "tst_test.h"
#include "tst_job.h"

static void start_test(struct tst_job *job)
{
	(void)job;
//	fprintf(stderr, "Starting test \e[1;37m%s\e[0m\n", job->test->name);
}

static void to_time(int *sec, int *nsec, struct timespec *start,
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

static void stop_test(struct tst_job *job)
{
	const char *name = job->test->name;
	int sec, nsec;

	to_time(&sec, &nsec, &job->start_time, &job->stop_time);

	switch (job->result) {
	case TST_SUCCESS:
		fprintf(stderr, "Test \e[1;37m%s\e[0m finished with "
		                "\e[1;32mSUCCESS\e[0m (duration %i.%05is)\n",
				name, sec, nsec);
	break;
	case TST_INTERR:
		fprintf(stderr, "Test \e[1;37m%s\e[0m finished with "
		                "\e[1;31mINTERNAL ERROR\e[0m (duration %i.%05is)\n",
		                name, sec, nsec);
	break;
	case TST_SIGSEGV:
		fprintf(stderr, "Test \e[1;37m%s\e[0m finished with "
		                "\e[1;31mSEGFAULT\e[0m (duration %i.%05is)\n",
				name, sec, nsec);
	break;
	case TST_TIMEOUT:
		fprintf(stderr, "Test \e[1;37m%s\e[0m finished with "
		                "\e[1;35mTIMEOUT\e[0m (duration %i.%05is}\n",
		                name, sec, nsec);
	break;
	case TST_MEMLEAK:
		fprintf(stderr, "Test \e[1;37m%s\e[0m finished with "
		                "\e[1;33mMEMLEAK\e[0m (duration %i.%05is}\n",
		                name, sec, nsec);
	break;
	case TST_FAILED:
		fprintf(stderr, "Test \e[1;37m%s\e[0m finished with "
		                "\e[1;31mFAILURE\e[0m (duration %i.%05is)\n",
				name, sec, nsec);
	break;
	}
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
		         "ommiting cleanup", path);
		return;
	}

	//TODO: Cleaner solution?
	char buf[256];
	snprintf(buf, sizeof(buf), "rm -rf '%s'", path);
	(void)system(buf);
}

/*
 * Create temp directory and cd into it
 */
static void create_tmpdir(const char *name, char *template, size_t size)
{
	/* Create template from test name */
	snprintf(template, size, "/tmp/ctest_%s_XXXXXX", name);

	/* Fix any funny characters in test name */
	char *s = template;

	while (*s != '\0') {
		if (!isalnum(*s) && *s != '/')
			*s = '_';
		s++;
	}

	if (mkdtemp(template) == NULL) {
		tst_warn("mkdtemp(%s) failed: %s", template, strerror(errno));
		exit(TST_INTERR);
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

static void child_write(struct tst_job *job, char ch)
{
	if (write(job->pipefd, &ch, 1) != 1)
		tst_warn("child write() failed: %s", strerror(errno));
}

void tst_job_run(struct tst_job *job)
{
	int ret;
	char template[256];
	int pipefd[2];

	/* Write down starting time of the test */
	clock_gettime(CLOCK_MONOTONIC, &job->start_time);

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
	break;
	default:
		close(pipefd[1]);
		job->pipefd = pipefd[0];
		job->running = 1;
		return;
	}

	/* Create directory in /tmp/ and chdir into it. */
	if (job->test->flags & TST_TMPDIR)
		create_tmpdir(job->test->name, template, sizeof(template));

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


	if (job->test->flags & TST_MALLOC_CHECK)
		tst_malloc_check_start();

	/* Run test */
	ret = job->test->tst_fn();

	if (job->test->flags & TST_MALLOC_CHECK) {
		size_t size;
		unsigned int chunks;
	
		tst_malloc_check_stop();
		
		tst_malloc_check_report(&size, &chunks);
	
		if (size != 0 && ret == TST_SUCCESS)
			ret = TST_MEMLEAK;
		
		//TODO message?
	}

	/* Send process cpu time to parent */
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &job->cpu_time);
	write_timespec(job, 'C', &job->cpu_time);

	/* Cleanup temporary dir recursively */
	if (job->test->flags & TST_TMPDIR)
		remove_tmpdir(template);

	/* Send the parent we are done */
	child_write(job, 'x');

	close(job->pipefd);

	exit(ret);
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
		default:
			job->result = TST_INTERR;
		}
	}
	
	/* Write down stop time  */
	clock_gettime(CLOCK_MONOTONIC, &job->stop_time);

	stop_test(job);
}
