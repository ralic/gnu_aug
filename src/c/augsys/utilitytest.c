/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

#define TEST_ "some test data"

int
main(int argc, char* argv[])
{
    char buf[64];
    if (!aug_autotlx())
        return 1;

    strcpy(buf, TEST_);
    aug_memfrob(buf, sizeof(buf) - 1);
    if (buf[0] != ('s' ^ 42) || 0 == strcmp(buf, TEST_)) {
       fprintf(stderr, "unexpected frob value\n");
       return 1;
    }
    aug_memfrob(buf, sizeof(buf) - 1);
    if (0 != strcmp(buf, TEST_)) {
       fprintf(stderr, "unexpected de-frob value\n");
       return 1;
    }
    return 0;
}
