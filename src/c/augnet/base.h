/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_BASE_H
#define AUGNET_BASE_H

#include "augnet/config.h"
#include "augnet/types.h"

struct aug_nbtype {
    int (*filecb_)(const struct aug_var*, struct aug_nbfile*,
                   struct aug_files*);
    int (*seteventmask_)(struct aug_nbfile*, unsigned short);
    int (*eventmask_)(struct aug_nbfile*);
    int (*events_)(struct aug_nbfile*);
    int (*shutdown_)(struct aug_nbfile*);
};

AUGNET_API const struct aug_nbfile*
aug_setnbfile(int fd, const struct aug_nbfile* nbfile);

AUGNET_API struct aug_nbfile*
aug_getnbfile(int fd, struct aug_nbfile* nbfile);

#endif /* AUGNET_BASE_H */
