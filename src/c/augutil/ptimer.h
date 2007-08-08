/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_PTIMER_H
#define AUGUTIL_PTIMER_H

#include "augutil/config.h"

typedef struct aug_ptimer_* aug_ptimer_t;

AUGUTIL_API aug_ptimer_t
aug_createptimer(void);

AUGUTIL_API int
aug_destroyptimer(aug_ptimer_t ptimer);

AUGUTIL_API int
aug_resetptimer(aug_ptimer_t ptimer);

/**
   Elapsed time is seconds.
*/

AUGUTIL_API int
aug_elapsed(aug_ptimer_t ptimer, double* secs);

#endif /* AUGUTIL_PTIMER_H */