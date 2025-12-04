// SPDX-License-Identifier: GPL-2.1-or-later
/*
 * Copyright (C) 2009-2013 Cyril Hrubis <metan@ucw.cz>
 */

#ifndef BOGOMAN_DEBUG_H
#define BOGOMAN_DEBUG_H

#define DEBUG(level, ...) \
        bogoman_dbg_print(level, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#define WARN(...) \
        bogoman_dbg_print(0, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)


void bogoman_dbg_print(unsigned int level, const char *file, const char *fn,
                       unsigned int line, const char *fmt, ...)
                       __attribute__ ((format (printf, 5, 6)));

void bogoman_set_dbg_level(unsigned int level);

#endif /* BOGOMAN_DEBUG_H */
