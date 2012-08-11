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

 /*
  
   Test job is an instance of running test.

  */

#ifndef TST_JOB_H
#define TST_JOB_H

#include <time.h>

#include "tst_msg.h"
#include "tst_preload.h"

struct tst_test;

struct tst_job {
	const struct tst_test *test;
	
	/* 
	 * Pipe fd.
	 *
	 * In parent this points to the read side of the pipe so the parent
	 * recieves data from child.
	 *
	 * In child this points to the write side of the pipe so child can
	 * send data to parent.
	 */
	int pipefd;
	
	int running:1;
	
	/* test execution time */
	struct timespec start_time;
	struct timespec stop_time;
	
	/* test cpu time */
	struct timespec cpu_time;

	/* test pid */
	int pid;
	
	/* test result */
	int result;

	/*
	 * test malloc statistics, filled if TST_MALLOC_CHECK was set.
	 */
	struct malloc_stats malloc_stats;

	/* store for test messages */
	struct tst_msg_store store;
};

/*
 * Runs a test job as a separate process.
 *
 * The test field must point to correct test.
 */
void tst_job_run(struct tst_job *job);

/*
 * Waits for the test to finish.
 */
void tst_job_wait(struct tst_job *job);

#endif /* TST_JOB_H */
