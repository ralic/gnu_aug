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
#include "augserv/base.h"
#include "augserv/options.h"
#include "augserv/types.h"

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

        aug_verify(flock_(&fl, fd, F_SETLKW, F_RDLCK));

        /* The lock has been obtained; daemon process must have stopped. */

        break;
    default:

        /* Invalid command. */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid control command [%d]"), (int)event);
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

AUGSERV_API aug_result
aug_start(const struct aug_options* options)
{
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ESUPPORT,
                   AUG_MSG("aug_start() not supported"));
    return AUG_FAILERROR;
}

AUGSERV_API aug_result
aug_control(const struct aug_options* options, int event)
{
    const char* pidfile;
    struct flock fl;
    int fd;
    aug_result result;

    aug_verify(aug_readservconf(AUG_CONFFILE(options), options->batch_,
                                AUG_TRUE));

    if (!(pidfile = aug_getservopt(AUG_OPTPIDFILE))) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("option 'AUG_OPTPIDFILE' not set"));
        return AUG_FAILERROR;
    }

    /* Check for existence of file. */

    if (-1 == access(pidfile, F_OK)) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EEXIST,
                       AUG_MSG("pidfile does not exist: %s"), pidfile);
        return AUG_FAILERROR;
    }

	if (-1 == (fd = open(pidfile, O_RDONLY)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    /* Attempt to obtain shared lock. */

    if (AUG_ISFAIL(flock_(&fl, fd, F_SETLK, F_RDLCK))) {

        /* As expected, the daemon process has an exclusive lock on the pid
           file.  Use F_GETLK to obtain the pid of the daemon process. */

        if (AUG_ISSUCCESS(result = flock_(&fl, fd, F_GETLK, F_RDLCK))) {

            /* Although a lock-manager allows locking over NFS, the returned
               pid is probably zero because it could be the pid of a process
               on another node. */

            if (0 == fl.l_pid) {

                aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EIO,
                               AUG_MSG("lockfile on NFS mount: %s"), pidfile);
                result = AUG_FAILERROR;

            } else
                result = send_(fd, fl.l_pid, event);

        }

    } else {

        /* The lock was obtained, therefore, the daemon process cannot be
           running. */

        if (-1 == unlink(pidfile)) {
            result = aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__,
                                         errno);
        } else {

            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EEXIST,
                           AUG_MSG("server process is not running"));
            result = AUG_FAILERROR;
        }
    }

    close(fd);
    return result;
}

AUGSERV_API aug_result
aug_install(const struct aug_options* options)
{
    return AUG_SUCCESS;
}

AUGSERV_API aug_result
aug_uninstall(const struct aug_options* options)
{
    return AUG_SUCCESS;
}