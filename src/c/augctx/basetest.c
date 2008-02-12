/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augctx.h"

#include <errno.h>

int
main(int argc, char* argv[])
{
    aug_initbasicctx();
    aug_setposixerrinfo(aug_geterrinfo(aug_tlx), __FILE__, __LINE__, ENOMEM);
    aug_perrinfo(aug_tlx, "operation failed");
    aug_term();
    return 0;
}