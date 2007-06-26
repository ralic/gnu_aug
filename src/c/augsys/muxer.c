/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/muxer.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if !defined(_WIN32)
# include "augsys/posix/muxer.c"
#else /* _WIN32 */
# include "augsys/win32/muxer.c"
#endif /* _WIN32 */