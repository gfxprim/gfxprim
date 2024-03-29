// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2012 Cyril Hrubis <metan@ucw.cz>
 */

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <math.h>

#include "tst_test.h"
#include "tst_job.h"
#include "tst_timespec.h"

/*
 * Once we child forks to do a job, this points to its job structure.
 */
static struct tst_job *my_job = NULL;

static int in_child(void)
{
	return my_job != NULL;
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
	char buf[310];
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
	char cmd[1024];
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
	snprintf(template, size, "/tmp/test_%s_XXXXXX", tmp);

	if (mkdtemp(template) == NULL) {
		tst_warn("mkdtemp(%s) failed: %s", template, strerror(errno));
		exit(TST_INTERR);
	}

	/*
	 * Copy resources if needed
	 *
	 * If resource is directory, copy only it's content.
	 */
	if (res_path != NULL) {
		struct stat st;
		char *p = "";

		if (stat(res_path, &st)) {
			tst_warn("failed to stat resource '%s': %s",
			         res_path, strerror(errno));
			rmdir(template);
			exit(TST_INTERR);
		}

		if (S_ISDIR(st.st_mode))
			p = "/*";

		snprintf(cmd, sizeof(cmd), "cp -r '%s'%s '%s'",
		         res_path, p, template);

		ret = system(cmd);

		if (ret) {
			tst_warn("failed to copy resource '%s'", res_path);
			rmdir(template);
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
	char buf[1 + sizeof(struct timespec)];
	char *ptr = buf;

	*(ptr++) = type;

	memcpy(ptr, time, sizeof(*time));

	if (write(job->pipefd, buf, sizeof(buf)) != sizeof(buf))
		tst_warn("write(timespec) failed: %s", strerror(errno));
}

/*
 * Reads timespec from pipe
 */
static void read_timespec(struct tst_job *job, struct timespec *time)
{
	int ret;

	do {
		ret = read(job->pipefd, time, sizeof(*time));
	} while (ret == 0);

	if (ret < 0 || ret != sizeof(*time))
		tst_warn("read(timespec) failed: %s", strerror(errno));
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

static int tst_vreport(int level, const char *fmt, va_list va)
{
	int ret;
	char buf[258];

	ret = vsnprintf(buf+3, sizeof(buf) - 3, fmt, va);

	ssize_t size = ret > 255 ? 255 : ret + 1;

	buf[0] = 'm';
	buf[1] = level;
	((unsigned char*)buf)[2] = size;

	if (in_child()) {
		if (write(my_job->pipefd, buf, size + 3) != size + 3)
			tst_warn("Failed to write msg to pipe.");
	} else {
		tst_warn("tst_report() called from parent, msg: '%s'",
		         buf + 3);
	}

	return ret;
}

int tst_report(int level, const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);
	ret = tst_vreport(level, fmt, va);
	va_end(va);

	return ret;
}

int tst_msg(const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);

	if (in_child())
		return tst_vreport(TST_MSG, fmt, va);

	fprintf(stderr, "MSG: ");
	ret = vfprintf(stderr, fmt, va);
	va_end(va);

	return ret;
}

int tst_warn(const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);

	if (in_child())
		return tst_vreport(TST_WARN, fmt, va);

	fprintf(stderr, "WARN: ");
	ret = vfprintf(stderr, fmt, va);
	va_end(va);

	return ret;
}

int tst_err(const char *fmt, ...)
{
	va_list va;
	int ret;

	va_start(va, fmt);

	if (in_child())
		return tst_vreport(TST_ERR, fmt, va);

	fprintf(stderr, "ERR: ");
	ret = vfprintf(stderr, fmt, va);
	va_end(va);

	return ret;
}

static int job_run(struct tst_job *job)
{
	int (*fn1)(void) = job->test->tst_fn;
	int (*fn2)(void*) = job->test->tst_fn;
	void *data = job->test->data;

	if (data)
		return fn2(data);

	return fn1();
}

/*
 * Run benchmark job and compute result
 */
static int tst_job_benchmark(struct tst_job *job)
{
	unsigned int i, iter = job->test->bench_iter;
	struct timespec cputime_start;
	struct timespec cputime_stop;
	struct timespec bench[iter];
	struct timespec sum = {.tv_sec = 0, .tv_nsec = 0};
	struct timespec dev = {.tv_sec = 0, .tv_nsec = 0};
	int ret;

	/* Warm up */
	ret = job_run(job);

	if (ret)
		return ret;

	/* Collect the data */
	for (i = 0; i < iter; i++) {
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cputime_start);

		ret = job_run(job);

		if (ret)
			return ret;

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cputime_stop);

		timespec_sub(&cputime_stop, &cputime_start, &bench[i]);

		timespec_add(&bench[i], &sum);
	}

	/* Compute mean */
	timespec_div(&sum, iter);

	double sum_d = timespec_to_double(&sum);
	double dev_d = 0;

	/* And standard deviation */
	for (i = 0; i < iter; i++) {
		double b = timespec_to_double(&bench[i]);

		b -= sum_d;
		b = b * b;

		dev_d += b;
	}

	dev_d /= iter;
	dev_d = sqrt(dev_d);

	double_to_timespec(dev_d, &dev);

	/* Send data to parent */
	write_timespec(job, 'M', &sum);
	write_timespec(job, 'V', &dev);

	return TST_PASSED;
}

void tst_job_run(struct tst_job *job)
{
	int ret;
	char template[300];
	int pipefd[2];

	/* Write down starting time of the test */
	clock_gettime(CLOCK_MONOTONIC, &job->start_time);

	/* Prepare the test message store */
	tst_msg_init(&job->store);

	/* copy benchmark interation */
	job->bench_iter = job->test->bench_iter;

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
//	if (freopen("/dev/null", "w", stderr) == NULL)
//		tst_warn("freopen(stderr) failed: %s", strerror(errno));

//	if (freopen("/dev/null", "w", stdout) == NULL)
//		tst_warn("freopen(stdout) failed: %s", strerror(errno));

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
	else
		alarm(300);

	/* Send process cpu time to parent */
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &job->cpu_time);
	write_timespec(job, 'c', &job->cpu_time);

	if (job->test->flags & TST_CHECK_MALLOC)
		tst_malloc_check_start();

	/* Run test */
	if (job->test->bench_iter) {
		ret = tst_job_benchmark(job);
	} else {
		if (job->test->flags & TST_MALLOC_CANARIES) {
			tst_malloc_canaries_set(MALLOC_CANARY_BEGIN);
			ret = job_run(job);

			if (!ret) {
				tst_malloc_canaries_set(MALLOC_CANARY_END);
				ret = job_run(job);
			}

			tst_malloc_canaries_set(MALLOC_CANARY_OFF);
		} else {
			ret = job_run(job);
		}
	}

	if (job->test->flags & TST_CHECK_MALLOC) {
		tst_malloc_check_stop();
		tst_malloc_check_report(&job->malloc_stats);

		child_write(job, 's', &job->malloc_stats,
                            sizeof(job->malloc_stats));

		if (job->malloc_stats.lost_chunks != 0 && ret == TST_PASSED)
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

void tst_job_read(struct tst_job *job)
{
	char ch;
	int ret;

	if (!job->running)
		tst_warn("job_read: Job %s (pid %i) not in running state",
		         job->test->name, job->pid);

	errno = 0;

	ret = read(job->pipefd, &ch, 1);

	if (ret < 0) {
		tst_warn("job_read: read() failed: %s", strerror(errno));
		job->running = 0;

		//TODO: kill the process?

		return;
	}

	/* Child exited => read returns end of file */
	if (ret == 0) {
		if (errno == EAGAIN) {
			tst_warn("job_read: read() returned EAGAIN");
			return;
		}

		job->running = 0;

		return;
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
	/* benchmark data */
	case 'M':
		read_timespec(job, &job->bench_mean);
	break;
	case 'V':
		read_timespec(job, &job->bench_var);
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
		tst_warn("parent: Invalid characters received");
	break;
	}
}

void tst_job_collect(struct tst_job *job)
{
	int status;

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
		 * Floating point exception, most likely
		 * division by zero (including integer division)
		 */
		case SIGFPE:
			job->result = TST_FPE;
		break;
		/*
		 * abort() called most likely double free or malloc data
		 * corruption
		 */
		case SIGABRT:
			job->result = TST_ABORTED;
		break;
		default:
			tst_warn("Test signaled with %i\n", WTERMSIG(status));
			job->result = TST_INTERR;
		}
	}

	/* Write down stop time  */
	clock_gettime(CLOCK_MONOTONIC, &job->stop_time);
}

void tst_job_wait(struct tst_job *job)
{
	while (job->running)
		tst_job_read(job);

	tst_job_collect(job);
}
