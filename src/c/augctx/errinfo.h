/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGCTX_ERRINFO_H
#define AUGCTX_ERRINFO_H

#include "augctx/config.h"

#include "augtypes.h" /* aug_bool */

#include <stdarg.h>

struct aug_errinfo {
    char file_[512];
    int line_;
    char src_[32];
    int num_;
    char desc_[512];
};

AUGCTX_API void
aug_clearerrinfo(struct aug_errinfo* errinfo);

AUGCTX_API void
aug_vseterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                const char* src, int num, const char* format, va_list args);

AUGCTX_API void
aug_seterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
               const char* src, int num, const char* format, ...);

/**
 * @return Exception code or zero.
 */

AUGCTX_API aug_result
aug_setposixerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    int num);

#if defined(_WIN32)

/**
 * @return Exception code or zero.
 */

AUGCTX_API aug_result
aug_setwin32errinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    unsigned long num);
#endif /* _WIN32 */

/**
 * Get equivalent errno value.
 *
 * @param errinfo Error info.
 *
 * @return Errno value or zero.
 */

AUGCTX_API int
aug_errno(const struct aug_errinfo* errinfo);

#endif /* AUGCTX_ERRINFO_H */
