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
#include <time.h>
#include <sys/time.h>
#include "TestUtils.h"
#include "TestHtml.h"

static FILE *outFile; 
static int   testCnt;

void Html_Init(void)
{
	outFile = fopen("results.html", "a");
	
	if (outFile == NULL) {
		//TODO
		TST_Exit();
	}

	testCnt = 0;

	fprintf(outFile, "   <tr><td colspan=\"3\" bgcolor=#bbbbff><b>%s - %s</b></td></tr>\n", TST_TestName, TST_TestDesc); 
}

static const char *line_color(void)
{
	if (testCnt % 2)
		return "#ccccee";
	else
		return "#ffffff";
}

void Html_Start(const char *CaseName)
{
	testCnt++;
	fprintf(outFile, "    <tr>\n     <td bgcolor=\"%s\">%s</td>\n", line_color(), CaseName);
}

static void print_time(struct timeval *time)
{
	struct tm r;

	gmtime_r(&time->tv_sec, &r);

	fprintf(outFile, "%02i:%02i:%02i.%04i",
	                 r.tm_hour,
	                 r.tm_min,
	                 r.tm_sec,
	                 (int)time->tv_usec/100);
}

void Html_Stop(int TST_flag, const char *format, va_list arg)
{
	const char *color, *res;

	if (TST_flag & TST_STAT) {
		fprintf(outFile, "     <td bgcolor=\"%s\">", line_color());
		print_time(&TST_TestData.Duration);
		fprintf(outFile, "</td>\n");
	} else
		fprintf(outFile, "     <td bgcolor=\"%s\"></td>\n", line_color());

	if (TST_flag & TST_FAIL) {
		color = "red";
		res  = "FAILED";
	} else {
		color = "green";
		res   = "SUCCEEDED";
	}

	fprintf(outFile, "     <td bgcolor=\"%s\"><font color=\"white\">%s</font></td>\n    </tr>\n", color, res);

	if (format == NULL)
		return;

	fprintf(outFile, "    <tr><td colspan=\"3\" bgcolor=\"#eeeeee\">\n     &nbsp;&nbsp;");
	vfprintf(outFile, format, arg);
	fprintf(outFile, "\n    </td>\n    </tr>\n");
}

void Html_Exit(void)
{
	const char *color;

	if (TST_TestData.fails == 0)
		color = "#aaffaa";
	else
		color = "ff9999";

	fprintf(outFile, "    <tr>\n     <td bgcolor=\"%s\"><b>Test Cases Executed = %i, Expected = %i, Failed = %i</b></td>\n",
	        color,
		TST_TestData.TestCases,
		TST_TestCases,
		TST_TestData.fails);
	
	
	fprintf(outFile, "     <td bgcolor=\"%s\"><b>", color);
	print_time(&TST_TestData.Duration);
	fprintf(outFile, "</b></td>\n    </tr>\n");
	
	fclose(outFile);
}
