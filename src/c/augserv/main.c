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
#define AUGSERV_BUILD
#include "augserv/main.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augserv/base.h"
#include "augserv/control.h"
#include "augserv/daemon.h"
#include "augserv/options.h"

#include "augutil/log.h"    /* aug_setdaemonlog() */

#include "augsys/utility.h" /* aug_perrinfo() */
#include "augsys/windows.h" /* GetStdHandle() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>         /* EXIT_FAILURE */

#define CONFFILE_(x) (*(x).confile_ ? (x).confile_ : NULL)

static jmp_buf mark_;

static void
die_(const char* s)
{
    aug_perrinfo(aug_tlx, s, NULL);
    longjmp(mark_, 1);
}

static void
foreground_(const struct aug_options* options)
{
    aug_result result;

    if (aug_readservconf(AUG_CONFFILE(options), options->batch_,
                         AUG_FALSE) < 0)
        die_("aug_readservconf() failed");

    if (aug_initserv_BIN() < 0)
        die_("aug_initserv() failed");

    result = aug_runserv();
    aug_termserv();
    if (result < 0)
        die_("aug_runserv() failed");
}

static aug_bool
daemonise_(const struct aug_options* options)
{
    if (aug_daemonise_BIN(options) < 0) {

        if (AUG_EXNONE == aug_getexcept(aug_tlx))
            return AUG_FALSE; /* Want foreground. */

        die_("aug_daemonise_BIN() failed");
    }

    return AUG_TRUE;
}

#if defined(_WIN32)
static void
start_(const struct aug_options* options)
{
    if (aug_start_N(options) < 0)
        die_("aug_start_N() failed");
}
#endif /* _WIN32 */

static void
control_(const struct aug_options* options, int sig)
{
    if (aug_control_BIN(options, sig) < 0)
        die_("aug_control_BIN() failed");
}

static void
install_(const struct aug_options* options)
{
    if (aug_install_N(options) < 0)
        die_("aug_install_N() failed");
}

static void
uninstall_(const struct aug_options* options)
{
    if (aug_uninstall(options) < 0)
        die_("aug_uninstall() failed");
}

AUGSERV_API int
aug_main(int argc, char* argv[], const struct aug_serv* serv)
{
    struct aug_options options;
    int jmpret = setjmp(mark_);
    if (jmpret)
        return jmpret;

    aug_setserv_(serv);

    if (aug_readopts_N(&options, argc, argv) < 0) {
        aug_perrinfo(aug_tlx, "getreadopts_N() failed", NULL);
        return EXIT_FAILURE;
    }

#if defined(_WIN32)

    /* Assume that if there is no stdin handle then the process is being
       started by the Service Control Manager.

       The arguments for Windows Services are specified after the "path to
       executable" in the Service Control Manager. */

    if (!GetStdHandle(STD_INPUT_HANDLE)) {

        /* Note: aug_readopts_N() will be called from the main service
           function. */

        if (daemonise_(&options))
            return EXIT_SUCCESS;

        /* Fallthrough to run in foreground. */
    }
#endif /* _WIN32 */

    switch (options.command_) {
    case AUG_CMDDEFAULT:
        foreground_(&options);
        break;
    case AUG_CMDEXIT:
        return EXIT_SUCCESS;
    case AUG_CMDINSTALL:
        aug_ctxinfo(aug_tlx, "installing daemon process");
        install_(&options);
        break;
    case AUG_CMDRECONF:
        aug_ctxinfo(aug_tlx, "re-configuring daemon process");
        control_(&options, AUG_EVENTRECONF);
        break;
    case AUG_CMDSTART:
#if !defined(_WIN32)
        /* Install daemon logger prior to opening log file. */

        aug_setdaemonlog(aug_tlx);
        daemonise_(&options);
#else /* _WIN32 */
        start_(&options);
#endif /* _WIN32 */
        break;
    case AUG_CMDSTATUS:
        aug_ctxinfo(aug_tlx, "getting status of daemon process");
        control_(&options, AUG_EVENTSTATUS);
        break;
    case AUG_CMDSTOP:
        aug_ctxinfo(aug_tlx, "stopping daemon process");
        control_(&options, AUG_EVENTSTOP);
        break;
    case AUG_CMDUNINSTALL:
        aug_ctxinfo(aug_tlx, "uninstalling daemon process");
        uninstall_(&options);
        break;
    }
    return EXIT_SUCCESS;
}
