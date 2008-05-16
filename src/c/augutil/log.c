/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/log.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/time.h"
#include "augsys/utility.h" /* aug_threadid() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/lock.h"

#include <assert.h>
#include <errno.h>          /* EINTR */
#include <stdio.h>

#if !defined(_WIN32)
# include <unistd.h>
#else /* _WIN32 */
# include <io.h>
# define snprintf _snprintf
# define vsnprintf _vsnprintf
#endif /* _WIN32 */

#if !defined(STDOUT_FILENO)
# define STDOUT_FILENO 1
#endif /* !STDOUT_FILENO */

#if !defined(STDERR_FILENO)
# define STDERR_FILENO 2
#endif /* !STDERR_FILENO */

/* The time format can defined by the build process if required. */

#if !defined(AUG_TIMEFORMAT)
# define AUG_TIMEFORMAT "%b %d %H:%M:%S"
#endif /* !AUG_TIMEFORMAT */

static const char* LABELS_[] = {
    "CRIT",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG"
};

static int
localtime_(struct tm* res)
{
    struct timeval tv;
    aug_clock* clock = aug_getclock(aug_tlx);
    aug_result result = aug_gettimeofday(clock, &tv);
    aug_release(clock);

    if (result < 0)
        return -1;

    if (!aug_localtime(&tv.tv_sec, res))
        return -1;

    return tv.tv_usec / 1000;
}

static int
writeall_(int fd, const char* buf, size_t n)
{
    /* Ensure all bytes are written and ignore any interrupts. */

    while (0 != n) {

#if !defined(_WIN32)
        int ret = write(fd, buf, n);
#else /* _WIN32 */
        int ret = write(fd, buf, (unsigned)n);
#endif /* _WIN32 */
        if (-1 == ret) {
            if (EINTR == errno)
                continue;

            aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
            return -1;
        }
        buf += ret, n -= ret;
    }
    return 0;
}

static void*
cast_(aug_log* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_logid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_log* obj)
{
}

static void
release_(aug_log* obj)
{
}

static aug_result
vwritelog_(aug_log* obj, int level, const char* format, va_list args)
{
    char buf[AUG_MAXLINE];
    size_t n = sizeof(buf);

    if (-1 == aug_vformatlog(buf, &n, level, format, args))
        return -1;

#if defined(_WIN32) && !defined(NDEBUG)
    aug_lock();
    OutputDebugString(buf);
    OutputDebugString("\n");
    aug_unlock();
#endif /* _WIN32 && !NDEBUG */

    buf[n] = '\n';
    return writeall_(level > AUG_LOGWARN ? STDOUT_FILENO : STDERR_FILENO,
                     buf, n + 1);
}

static const struct aug_logvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    vwritelog_
};

static aug_log daemonlog_ = { &vtbl_, NULL };


AUGUTIL_API const char*
aug_loglabel(int level)
{
    if (sizeof(LABELS_) / sizeof(LABELS_[0]) <= (size_t)level)
        level = AUG_LOGDEBUG0;

    return LABELS_[level];
}

AUGUTIL_API int
aug_vformatlog(char* buf, size_t* n, int level, const char* format,
               va_list args)
{
    int ms, ret;
    size_t size;
    struct tm tm;

    /* At least one character is needed for the null-terminator. */

    assert(buf && n && *n && format);
    size = *n;

    if (-1 == (ms = localtime_(&tm)))
        return -1;

    /* The return value from the strftime() function is either a) the number
       of characters copied to the buffer, excluding the null terminator, or
       b) zero, indicating an error. */

    if (0 == (ret = (int)strftime(buf, size, AUG_TIMEFORMAT, &tm)))
        goto done;

    buf += ret, size -= ret;

    /* Null termination is _not_ guaranteed by snprintf(). */

#if ENABLE_THREADS
    ret = snprintf(buf, size, ".%03d %08x %-6s ", ms, aug_threadid(),
                   aug_loglabel(level));
#else /* !ENABLE_THREADS */
    ret = snprintf(buf, size, ".%03d %-6s ", ms, aug_loglabel(level));
#endif /* !ENABLE_THREADS */

    AUG_SNTRUNCF(buf, size, ret);

    if (ret < 0) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    buf += ret, size -= ret;

    ret = vsnprintf(buf, size, format, args);
    AUG_SNTRUNCF(buf, size, ret);

    if (ret < 0) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

 done:

    buf += ret, size -= ret;
    *buf = '\0';

    /* Set output parameter to be total number of characters copied. */

    *n -= size;
    return 0;
}

AUGUTIL_API int
aug_formatlog(char* buf, size_t* n, int level, const char* format, ...)
{
    int ret;
    va_list args;

    va_start(args, format);
    ret = aug_vformatlog(buf, n, level, format, args);
    va_end(args);

    return ret;
}

AUGUTIL_API aug_log*
aug_getdaemonlog(void)
{
    return &daemonlog_;
}
