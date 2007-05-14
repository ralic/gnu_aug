/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   TODO
   \file body.h
 */
#ifndef AUGMAR_BODY_H_
#define AUGMAR_BODY_H_

#include "augmar/seq_.h"

struct aug_info_;

AUG_EXTERN int
aug_setcontent_(aug_seq_t seq, struct aug_info_* info, const void* data,
                unsigned size);

AUG_EXTERN int
aug_truncate_(aug_seq_t seq, struct aug_info_* info, unsigned size);

AUG_EXTERN int
aug_write_(aug_seq_t seq, struct aug_info_* info, unsigned offset,
           const void* buf, unsigned len);

AUG_EXTERN const void*
aug_content_(aug_seq_t seq, const struct aug_info_* info);

AUG_EXTERN int
aug_read_(aug_seq_t seq, const struct aug_info_* info, unsigned offset,
          void* buf, unsigned len);

#endif /* AUGMAR_BODY_H_ */
