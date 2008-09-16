/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv/base.h"
#include "augsrv/options.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include "augext/log.h"

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>          /* struct flock */
#include <signal.h>         /* kill() */
#include <strings.h>        /* bzero() */
#include <unistd.h>

static aug_result
flock_(struct flock* fl, int fd, int cmd, int type)
{
    bzero(fl, sizeof(*fl));

    fl->l_type = type;
    fl->l_whence = SEEK_SET;
    fl->l_start = 0;
    fl->l_len = 0;

    if (-1 == fcntl(fd, cmd, fl))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

static aug_result
send_(int fd, pid_t pid, int event)
{
    struct flock fl;

    switch (event) {
    case AUG_EVENTRECONF:
        if (-1 == kill(pid, SIGHUP))
            return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        break;
    case AUG_EVENTSTATUS:
        if (-1 == kill(pid, SIGUSR1))
            return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        break;
    case AUG_EVENTSTOP:
        if (-1 == kill(pid, SIGTERM))
            return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

        /* Wait for daemon process to release lock. */

        if (-1 == flock_(&fl, fd, F_SETLKW, F_RDLCK))
            return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

        /* The lock has been obtained; daemon process must have stopped. */

        break;
    default:

        /* Invalid command. */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid control command '%d'"), (int)event);
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

AUGSRV_API int
aug_start(void)
{
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                   AUG_MSG("aug_start() not supported"));
    return -1;
}

AUGSRV_API int
aug_control(int event)
{
    const char* pidfile;
    struct flock fl;
    int fd, ret = -1;

    if (!(pidfile = aug_getserviceopt(AUG_OPTPIDFILE))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTPIDFILE' not set"));
        return -1;
    }

    /* Check for existence of file. */

    if (-1 == access(pidfile, F_OK)) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EEXIST,
                       AUG_MSG("pidfile does not exist: %s"), pidfile);
        return -1;
    }

	if (-1 == (fd = open(pidfile, O_RDONLY))) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }

    /* Attempt to obtain shared lock. */

    if (-1 == flock_(&fl, fd, F_SETLK, F_RDLCK)) {

        /* As expected, the daemon process has an exclusive lock on the pid
           file.  Use F_GETLK to obtain the pid of the daemon process. */

        if (-1 == flock_(&fl, fd, F_GETLK, F_RDLCK))
            goto done;

        /* Although a lock-manager allows locking over NFS, the returned pid
           is probably zero because it could be the pid of a process on
           another node. */

        if (0 == fl.l_pid) {
            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EIO,
                           AUG_MSG("lockfile on NFS mount: %s"), pidfile);
            return -1;
        }

        if (-1 == send_(fd, fl.l_pid, event))
            goto done;

        ret = 0;

    } else {

        /* The lock was obtained, therefore, the daemon process cannot be
           running. */

        if (-1 == unlink(pidfile)) {
            aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
            goto done;
        }

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EEXIST,
                       AUG_MSG("server process is not running"));
        ret = -1;
    }

 done:
    close(fd);
    return ret;
}

AUGSRV_API int
aug_install(void)
{
    return 0;
}

AUGSRV_API int
aug_uninstall(void)
{
    return 0;
}
