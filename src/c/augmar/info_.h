/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   TODO
   \file info.h
 */

#ifndef AUGMAR_INFO_H_
#define AUGMAR_INFO_H_

#include "augmar/format_.h"
#include "augmar/seq_.h"

struct aug_info_ {
    unsigned verno_, fields_, hsize_, bsize_;
};

AUG_EXTERN int
aug_setinfo_(aug_seq_t seq, const struct aug_info_* info);

AUG_EXTERN int
aug_info_(aug_seq_t seq, struct aug_info_* info);

#endif /* AUGMAR_INFO_H_ */
