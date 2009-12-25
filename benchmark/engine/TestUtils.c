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
 * Copyright (C) 2009-2010 Jiri "BlueBear" Dluhos                            *
 *                         <jiri.bluebear.dluhos@gmail.com>                  *
 *                                                                           *
 * Copyright (C) 2009-2010 Cyril Hrubis <metan@ucw.cz>                       *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include "TestUtils.h"
#include "TestHtml.h"

#define LINE_SIZE 50
#define OutFile stdout

struct TST_TestData TST_TestData;

static void PutChars(char ch, unsigned int nr)
{
	while (nr-- > 0)
		fputc(ch, OutFile);
}

static void CenterName(const char *name)
{
	size_t len = strlen(name);
	int skip = (LINE_SIZE - len - 2) / 2;

	PutChars('-', skip);
	fprintf(OutFile, " %s ", name);
	PutChars('-', LINE_SIZE - 2 - len - skip);
	fputc('\n', OutFile);
}

static void PrintTime(struct timeval *time)
{
	struct tm result;
	
	gmtime_r(&time->tv_sec, &result);

	fprintf(OutFile, "%02i:%02i:%02i.%04li", 
		result.tm_hour,
		result.tm_min,
		result.tm_sec,
		time->tv_usec/100);
}

#define MAX_SIGNALS 10
static int signals[MAX_SIGNALS] = {
 SIGHUP, SIGINT, SIGQUIT, SIGTRAP, SIGABRT,
 SIGBUS, SIGFPE, SIGKILL, SIGSEGV, SIGTERM
}; 

static void sighandler(int sig)
{
	TST_Stop(TST_FAIL | TST_EXIT, "got signal (%i) `%s'", sig, sys_siglist[sig]);
}

void TST_Init(void)
{
	int i;

	for (i = 0; i < MAX_SIGNALS; i++)
		signal(signals[i], sighandler);
	
	CenterName(TST_TestName);
	fprintf(OutFile, "%s\n\n", TST_TestDesc);
	TST_TestData.fails = 0;
	TST_TestData.TestCases = 0;

	Html_Init();

	gettimeofday(&TST_TestData.TestStartTime, NULL);
}

void TST_Start(const char *CaseName)
{
	size_t len = strlen(CaseName);
	
	TST_TestData.TestCases++;

	Html_Start(CaseName);

	fprintf(OutFile, "%s ", CaseName);
	PutChars('.', LINE_SIZE - len - 5);
	
	if (OutFile == stdout)
		fflush(OutFile);
	
	gettimeofday(&TST_TestData.StartTime, NULL);
}

void TST_Stop(int TST_flag, const char *format, ...)
{
	struct timeval now;
	va_list arg;
	
	if (TST_flag & TST_STAT) {
		gettimeofday(&now, NULL);
		timersub(&now, &TST_TestData.StartTime, &TST_TestData.Duration);
	}

	va_start(arg, format);
	Html_Stop(TST_flag, format, arg);
	va_end(arg);

	if (TST_flag & TST_FAIL) {
		TST_TestData.fails++;
		fprintf(OutFile, " ERR\n");
	} else
		fprintf(OutFile, "  OK\n");

	if (format != NULL) {
		fprintf(OutFile, " (");
		va_start(arg, format);
		vfprintf(OutFile, format, arg);
		va_end(arg);
		fprintf(OutFile, ")\n");
	}

	if (TST_flag & TST_STAT) {
		fprintf(OutFile, " (Consumed time: ");
		PrintTime(&TST_TestData.Duration);
		fprintf(OutFile, ")\n");
	}

	if (TST_flag & TST_EXIT)
		TST_Exit();

}

void TST_Exit(void)
{
	struct timeval now;

	gettimeofday(&now, NULL);
	timersub(&now, &TST_TestData.TestStartTime, &TST_TestData.Duration);

	fprintf(OutFile, "\nTest duration: ");
	PrintTime(&TST_TestData.Duration);
	fprintf(OutFile, "\nTest cases expected: %i executed: %i failed: %i\n",
	                 TST_TestCases,
	                 TST_TestData.TestCases,
	                 TST_TestData.fails);

	PutChars('-', LINE_SIZE);
	fprintf(OutFile, "\n");
	
	Html_Exit();

	if (TST_TestData.fails > 0)
		exit(EXIT_FAILURE);
	else
		exit(EXIT_SUCCESS);
}
