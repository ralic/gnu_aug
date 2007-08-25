/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/log.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/base.h" /* aug_loglevel() */
#include "augsys/lock.h"

#if defined(_WIN32)
# include "augsys/windows.h"
#endif /* _WIN32 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>

#if defined(_WIN32)
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

AUGSYS_API int
aug_stdiologger(int loglevel, const char* format, va_list args)
{
    char buf[AUG_MAXLINE];
    FILE* file = loglevel > AUG_LOGWARN ? stdout : stderr;

    if (0 > vsnprintf(buf, sizeof(buf), format, args)) {
        errno = EINVAL;
        return -1;
    }

#if defined(_WIN32) && !defined(NDEBUG)
    aug_lock();
    OutputDebugString(buf);
    OutputDebugString("\n");
    aug_unlock();
#endif /* _WIN32 && !NDEBUG */

    fprintf(file, "%s\n", buf);
    fflush(file);
    return 0;
}

AUGSYS_API int
aug_setloglevel(int loglevel)
{
    return aug_setloglevel_(loglevel);
}

AUGSYS_API aug_logger_t
aug_setlogger(aug_logger_t logger)
{
    logger = aug_setlogger_(logger);
    return logger ? logger : aug_stdiologger;
}

AUGSYS_API int
aug_loglevel(void)
{
    return aug_loglevel_(NULL);
}

AUGSYS_API aug_logger_t
aug_logger(void)
{
    aug_logger_t logger;
    aug_loglevel_(&logger);
    return logger ? logger : aug_stdiologger;
}

AUGSYS_API int
aug_vwritelog(int loglevel, const char* format, va_list args)
{
    aug_logger_t logger;
    assert(format);
    if (aug_loglevel_(&logger) < loglevel)
        return 0;

    /* Fallback to default. */

    if (!logger)
        logger = aug_stdiologger;

    return (*logger)(loglevel, format, args);
}

AUGSYS_API int
aug_writelog(int loglevel, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(loglevel, format, args);
    va_end(args);
    return ret;
}

AUGSYS_API int
aug_crit(const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(AUG_LOGCRIT, format, args);
    va_end(args);
    return ret;
}

AUGSYS_API int
aug_error(const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(AUG_LOGERROR, format, args);
    va_end(args);
    return ret;
}

AUGSYS_API int
aug_warn(const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(AUG_LOGWARN, format, args);
    va_end(args);
    return ret;
}

AUGSYS_API int
aug_notice(const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(AUG_LOGNOTICE, format, args);
    va_end(args);
    return ret;
}

AUGSYS_API int
aug_info(const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(AUG_LOGINFO, format, args);
    va_end(args);
    return ret;
}

AUGSYS_API int
aug_debug0(const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(AUG_LOGDEBUG0, format, args);
    va_end(args);
    return ret;
}

AUGSYS_API int
aug_debug1(const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(AUG_LOGDEBUG0 + 1, format, args);
    va_end(args);
    return ret;
}

AUGSYS_API int
aug_debug2(const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(AUG_LOGDEBUG0 + 2, format, args);
    va_end(args);
    return ret;
}

AUGSYS_API int
aug_debug3(const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = aug_vwritelog(AUG_LOGDEBUG0 + 3, format, args);
    va_end(args);
    return ret;
}
