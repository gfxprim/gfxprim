// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2009-2025 Cyril Hrubis <metan@ucw.cz>
 */

 /**
  * @file gp_io_parser.h
  * @brief Commmon functions to parse data.
  */

#ifndef LOADERS_GP_IO_PARSER_H
#define LOADERS_GP_IO_PARSER_H

#include <loaders/gp_io.h>

/**
 * @brief Reads unicode string stored in big endian utf16 format.
 *
 * @param io A readable I/O.
 * @param nbytes Number of characters, the number of bytes is twice the number
 *               of the characters for utf16.
 *
 * @return A newly allocated null terminated utf8 string or NULL in a case of a
 *         allocation failure.
 */
char *gp_io_read_b2_utf16(gp_io *io, size_t nchars);

#endif /* LOADERS_GP_IO_PARSER_H */
