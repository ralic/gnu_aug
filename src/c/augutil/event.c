/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/event.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/object.h"

#include "augsys/barrier.h"
#include "augsys/base.h" /* aug_getosfd() */
#include "augsys/muxer.h"

#include "augctx/errinfo.h"
#include "augctx/errno.h"

#include <errno.h>
#include <signal.h>

#if defined(_WIN32)
# define SIGHUP  1
# define SIGUSR1 10
#endif /* _WIN32 */

static int
readall_(aug_md md, char* buf, size_t n)
{
    /* Ensure all bytes are read and ignore any interrupts. */

    while (0 != n) {

        int ret = aug_mread(md, buf, n);
        if (-1 == ret) {
            if (EINTR == aug_errno())
                continue;

            return -1;
        }
        buf += ret, n -= ret;
    }
    return 0;
}

static int
writeall_(aug_md md, const char* buf, size_t n)
{
    /* Ensure all bytes are written and ignore any interrupts. */

    while (0 != n) {

        int ret = aug_mwrite(md, buf, n);
        if (-1 == ret) {
            if (EINTR == aug_errno())
                continue;

            return -1;
        }
        buf += ret, n -= ret;
    }
    return 0;
}

AUGUTIL_API struct aug_event*
aug_setsigevent(struct aug_event* event, int sig)
{
    switch (sig) {
    case SIGHUP:
        event->type_ = AUG_EVENTRECONF;
        break;
    case SIGUSR1:
        event->type_ = AUG_EVENTSTATUS;
        break;
    case SIGINT:
    case SIGTERM:
        event->type_ = AUG_EVENTSTOP;
        break;
    default:
        event->type_ = AUG_EVENTSIGNAL;
    }
    event->ob_ = (aug_object*)aug_createlongob(sig, NULL);
    return event;
}

AUGUTIL_API struct aug_event*
aug_readevent(aug_md md, struct aug_event* event)
{
    if (-1 == readall_(md, (char*)event, sizeof(*event)))
        return NULL;

    /* Ensure writes are visible: ensure that any components of the event
       object are read from main memory. */

    AUG_RMB();

    return event;
}

AUGUTIL_API const struct aug_event*
aug_writeevent(aug_md md, const struct aug_event* event)
{
    /* Flush pending writes to main memory: ensure that the event object is
       visible to other threads. */

    AUG_WMB();

    /* Must increment before write. */

    if (event->ob_)
        aug_retain(event->ob_);

    if (-1 == writeall_(md, (const char*)event, sizeof(*event))) {
        if (event->ob_)
            aug_release(event->ob_);
        return NULL;
    }

    return event;
}
