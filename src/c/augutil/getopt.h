/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGUTIL_GETOPT_H
#define AUGUTIL_GETOPT_H

/**
 * @file augutil/getopt.h
 * @author Henry Spencer
 *
 * Get option letter from argv.
 */

#include "augutil/config.h"

#define aug_optarg aug_optarg_()
#define aug_optind (*aug_optind_())
#define aug_opterr (*aug_opterr_())
#define aug_optopt (*aug_optopt_())

AUGUTIL_API char*
aug_optarg_(void);

AUGUTIL_API int*
aug_optind_(void);

AUGUTIL_API int*
aug_opterr_(void);

AUGUTIL_API int*
aug_optopt_(void);

AUGUTIL_API int
aug_getopt(int argc, char** argv, const char* optstring);

#endif /* AUGUTIL_GETOPT_H */
