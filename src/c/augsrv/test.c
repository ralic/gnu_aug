/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsrv.h"
#include "augutil.h"
#include "augsys.h"

#include <stdio.h>

static const char* program_;
static char conffile_[AUG_PATH_MAX + 1] = "";
static int daemon_ = 0;

static const char*
getopt_(const struct aug_var* arg, enum aug_option opt)
{
    switch (opt) {
    case AUG_OPTCONFFILE:
        return *conffile_ ? conffile_ : NULL;
    case AUG_OPTEMAIL:
        return "Mark Aylett <mark@emantic.co.uk>";
    case AUG_OPTLONGNAME:
        return "Test Program";
    case AUG_OPTPIDFILE:
        return "test.pid";
    case AUG_OPTPROGRAM:
        return program_;
    case AUG_OPTSHORTNAME:
        return "test";
    }
    return NULL;
}

static int
config_(const struct aug_var* arg, const char* conffile, int daemon)
{
    if (conffile && !aug_realpath(conffile_, conffile, sizeof(conffile_)))
        return -1;

    daemon_ = daemon;
    return 0;
}

static int
init_(const struct aug_var* arg)
{
    return 0;
}

static int
run_(const struct aug_var* arg)
{
    struct aug_event in = { 1, AUG_VARNULL }, out = { !1, AUG_VARNULL };

    if (!aug_writeevent(aug_eventout(), &in))
        return -1;

    if (!aug_readevent(aug_eventin(), &out))
        return -1;

    if (in.type_ != out.type_) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EIO,
                       AUG_MSG("unexpected event type from"
                               " aug_readevent()"));
        return -1;
    }

    return 0;
}

static void
term_(const struct aug_var* arg)
{
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    struct aug_service service = {
        getopt_,
        config_,
        init_,
        run_,
        term_,
        AUG_VARNULL
    };

    program_ = argv[0];

    if (-1 == aug_atexitinit(&errinfo)) {
        aug_perror("aug_atexitinit() failed");
        return 1;
    }

    return aug_main(&service, argc, argv);
}
