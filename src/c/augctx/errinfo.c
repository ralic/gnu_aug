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
#define AUGCTX_BUILD
#include "augctx/errinfo.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/errno.h"
#include "augctx/string.h"

#include <errno.h>
#include <stdio.h>

#if defined(_WIN32)
# if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
# endif /* !WIN32_LEAN_AND_MEAN */
# include <windows.h>
# include <ctype.h>
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

static void
seterrinfo_(struct aug_errinfo* errinfo, const char* file, int line,
            const char* src, int num, const char* desc)
{
    aug_strlcpy(errinfo->file_, file, sizeof(errinfo->file_));
    errinfo->line_ = line;
    aug_strlcpy(errinfo->src_, src, sizeof(errinfo->src_));
    errinfo->num_ = num;
    aug_strlcpy(errinfo->desc_, desc, sizeof(errinfo->desc_));
}

static void
vseterrinfo_(struct aug_errinfo* errinfo, const char* file, int line,
             const char* src, int num, const char* format, va_list args)
{
    int ret;

    aug_strlcpy(errinfo->file_, file, sizeof(errinfo->file_));
    errinfo->line_ = line;
    aug_strlcpy(errinfo->src_, src, sizeof(errinfo->src_));
    errinfo->num_ = num;

    /* Null termination is _not_ guaranteed by snprintf(). */

    ret = vsnprintf(errinfo->desc_, sizeof(errinfo->desc_), format, args);
    AUG_SNTRUNCF(errinfo->desc_, sizeof(errinfo->desc_), ret);

    if (ret < 0)
        aug_strlcpy(errinfo->desc_, "no message - bad format",
                    sizeof(errinfo->desc_));
}

AUGCTX_API void
aug_clearerrinfo(struct aug_errinfo* errinfo)
{
    errinfo->file_[0] = '\0';
    errinfo->line_ = 0;
    errinfo->src_[0] = '\0';
    errinfo->num_ = 0;
    errinfo->desc_[0] = '\0';
}

AUGCTX_API void
aug_clearctxerror(aug_ctx* ctx)
{
    aug_setexcept(ctx, 0);
    aug_clearerrinfo(aug_geterrinfo(ctx));
}

AUGCTX_API void
aug_vseterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                const char* src, int num, const char* format, va_list args)
{
    vseterrinfo_(errinfo, file, line, src, num, format, args);
}

AUGCTX_API void
aug_vsetctxerror(aug_ctx* ctx, const char* file, int line, const char* src,
                 int num, const char* format, va_list args)
{
    aug_setexcept(ctx, AUG_EXERROR);
    vseterrinfo_(aug_geterrinfo(ctx), file, line, src, num, format, args);
}

AUGCTX_API void
aug_seterrinfo(struct aug_errinfo* errinfo, const char* file, int line,
               const char* src, int num, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vseterrinfo_(errinfo, file, line, src, num, format, args);
    va_end(args);
}

AUGCTX_API void
aug_setctxerror(aug_ctx* ctx, const char* file, int line, const char* src,
                int num, const char* format, ...)
{
    va_list args;
    aug_setexcept(ctx, AUG_EXERROR);
    va_start(args, format);
    vseterrinfo_(aug_geterrinfo(ctx), file, line, src, num, format, args);
    va_end(args);
}

AUGCTX_API unsigned
aug_setposixerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    int num)
{
    seterrinfo_(errinfo, file, line, "posix", num, strerror(num));

    /* Map to exception code. */

    switch (num) {
    case EINTR:
        return AUG_EXINTR;
    case ENOENT:
        return AUG_EXNONE;
    case EWOULDBLOCK:
        return AUG_EXBLOCK;
    }
    return AUG_EXERROR;
}

AUGCTX_API void
aug_setposixerror(aug_ctx* ctx, const char* file, int line, int num)
{
    aug_setexcept(ctx, aug_setposixerrinfo(aug_geterrinfo(ctx),
                                           file, line, num));
}

#if defined(_WIN32)
AUGCTX_API unsigned
aug_setwin32errinfo(struct aug_errinfo* errinfo, const char* file, int line,
                    unsigned long num)
{
    char desc[AUG_MAXLINE];
    DWORD i = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, num,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), desc,
                            sizeof(desc), NULL);

    /* Remove trailing whitespace. */

    while (i && isspace(desc[i - 1]))
        --i;

    /* Remove trailing full-stop. */

    if (i && '.' == desc[i - 1])
        --i;

    desc[i] = '\0';

    seterrinfo_(errinfo, file, line, "win32", (int)num,
                i ? desc : AUG_MSG("no description available"));

    /* Map to exception code. */

    switch (aug_win32posix(num)) {
    case EINTR:
        return AUG_EXINTR;
    case ENOENT:
        return AUG_EXNONE;
    case EWOULDBLOCK:
        return AUG_EXBLOCK;
    }
    return AUG_EXERROR;
}

AUGCTX_API void
aug_setwin32error(aug_ctx* ctx, const char* file, int line, unsigned long num)
{
    aug_setexcept(ctx, aug_setwin32errinfo(aug_geterrinfo(ctx),
                                           file, line, num));
}
#endif /* _WIN32 */

AUGCTX_API int
aug_errno(const struct aug_errinfo* errinfo)
{
    if (0 == aug_strncasecmp(errinfo->src_, "posix", sizeof(errinfo->src_)))
        return errinfo->num_;

#if defined(_WIN32)
    if (0 == aug_strncasecmp(errinfo->src_, "win32", sizeof(errinfo->src_)))
        return aug_win32posix(errinfo->num_);
#endif /* _WIN32 */

    return 0;
}
