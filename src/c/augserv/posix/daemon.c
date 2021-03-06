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

#include "augutil/log.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/string.h"

#include <assert.h>
#if HAVE_ALLOCA_H
# include <alloca.h>
#endif /* HAVE_ALLOCA_H */
#include <fcntl.h>          /* struct flock */
#include <math.h>           /* log10() */
#include <stdio.h>          /* snprintf() */
#include <strings.h>        /* bzero() */
#include <unistd.h>         /* getdtablesize() */
#include <sys/types.h>      /* umask() */
#include <sys/stat.h>       /* umask() */

static void
closeall_(int next)
{
    int limit = getdtablesize();
    for (; next <= limit; ++next)
        close(next);
}

static aug_result
daemonise_B_(void)
{
    /* EXCEPT: daemonise_B_ -> fork; */
    /* EXCEPT: fork -> EAGAIN; */
    switch (fork()) {
    case 0:
        break;
    case -1:
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    default:
        /* Use system version of exit to avoid flushing standard
           streams. */
        _exit(0);
    }

    /* Detach from controlling terminal by making process a session
       leader. */

    if (setsid() < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    /* Forking again ensures that the daemon process is not a session leader,
       and therefore cannot regain access to a controlling terminal. */

    /* EXCEPT: daemonise_B_ -> fork; */
    /* EXCEPT: fork -> EAGAIN; */
    switch (fork()) {
    case 0:
        break;
    case -1:
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    default:
        /* Use system version of exit to avoid flushing standard streams. */
        _exit(0);
    }

    /* Restrict file creation mode. */
    /* No need to check the return value: it is the previous value of the file
       mode mask. This function is always successful */
    umask(0027);

    /* Leave both the standard file descriptors (3), and the signal pipe
       descriptors (2) open. */

    closeall_(3 + 2);
    return 0;
}

size_t
digits_(unsigned long n)
{
    return 0 == n ? 1 : ((int)log10((double)n)) + 1;
}

static aug_result
writepid_BI_(int fd)
{
    pid_t pid = getpid();
    size_t len = digits_(pid) + 1; /* One for newline. */
    char* str = alloca(sizeof(char) * (len + 1));
    if (!str) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, ENOMEM);
        return -1;
    }

    /* Resulting buffer will _not_ be null terminated. */

    if (len != (snprintf(str, len + 1, "%ld\n", (long)pid))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EFORMAT,
                        AUG_MSG("pid formatting failed"));
        return -1;
    }

    /* EXCEPT: writepid_BI_ -> write; */
    /* EXCEPT: write -> EAGAIN; */
    /* EXCEPT: write -> EINTR; */
    if (len != write(fd, str, len) || fsync(fd) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static aug_result
flock_BI_(struct flock* fl, int fd, int cmd, int type)
{
    bzero(fl, sizeof(*fl));

    fl->l_type = type;
    fl->l_whence = SEEK_SET;
    fl->l_start = 0;
    fl->l_len = 0;

    /* EXCEPT: flock_BI_ -> fcntl; */
    /* EXCEPT: fcntl -> EAGAIN; */
    /* EXCEPT: fcntl -> EINTR; */
    if (fcntl(fd, cmd, fl) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static aug_result
lockfile_BIN_(const char* path)
{
    struct flock fl;
    int fd;

    /* EXCEPT: lockfile_BIN_ -> open; */
    /* EXCEPT: open -> ENOENT; */
    if ((fd = open(path, O_CREAT | O_WRONLY, 0640)) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    /* Attempt to obtain exclusive lock. */

    /* EXCEPT: lockfile_BIN_ -> flock_BI_; */

    if (flock_BI_(&fl, fd, F_SETLK, F_WRLCK) < 0) {

        if (AUG_EXBLOCK == aug_getexcept(aug_tlx)) {

            /* EWOULDBLOCK indicates that another process has locked the
               file. */

            aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EEXIST,
                            AUG_MSG("pidfile still in use: %s"), path);
        }
        goto fail;
    }

    /* Truncate any existing pid value. */

    /* EXCEPT: lockfile_BIN_ -> ftruncate; */
    /* EXCEPT: ftruncate -> EAGAIN; */
    /* EXCEPT: ftruncate -> EINTR; */
    if (ftruncate(fd, 0) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        goto fail;
    }

    /* EXCEPT: lockfile_BIN_ -> writepid_BI_; */

    if (writepid_BI_(fd) < 0)
        goto fail;

    /* Success: do not close the file - this would release the lock. */

    return 0;

 fail:
    close(fd);
    return -1;
}

static aug_result
closein_IN_(void)
{
    int fd;
    aug_result result;

    /* EXCEPT: closein_IN_ -> open; */
    /* EXCEPT: open -> ENOENT; */
	if ((fd = open("/dev/null", O_RDONLY)) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    /* EXCEPT: closein_IN_ -> dup2; */
    /* EXCEPT: dup2 -> EINTR; */
    if (dup2(fd, STDIN_FILENO) < 0) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        result = -1;
    } else
        result = 0;

    close(fd);
    return result;
}

AUGSERV_API aug_result
aug_daemonise_BIN_(const struct aug_options* options)
{
    const char* pidfile;
    aug_result result;

    /* Install daemon logger prior to opening log file. */

    aug_setdaemonlog(aug_tlx);

    if (aug_readservconf(AUG_CONFFILE(options), options->batch_,
                         AUG_TRUE) < 0)
        return -1;

    if (!(pidfile = aug_getservopt(AUG_OPTPIDFILE))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EINVAL,
                        AUG_MSG("option 'AUG_OPTPIDFILE' not set"));
        return -1;
    }

    /* EXCEPT: aug_daemonise_BIN_ -> daemonise_B_; */
    /* EXCEPT: aug_daemonise_BIN_ -> lockfile_BIN_; */
    /* EXCEPT: aug_daemonise_BIN_ -> closein_IN_; */
    /* EXCEPT: aug_daemonise_BIN_ -> aug_initserv_BIN; */

    if (daemonise_B_() < 0 || lockfile_BIN_(pidfile) < 0
        || closein_IN_() < 0 || aug_initserv_BIN() < 0)
        return -1;

    result = aug_runserv();
    aug_termserv();
    return result;
}
