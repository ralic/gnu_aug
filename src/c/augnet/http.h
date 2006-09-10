/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_HTTP_H
#define AUGNET_HTTP_H

#include "augnet/config.h"

#include "augsys/types.h"

struct aug_var;

struct aug_httphandlers {
    void (*setinitial_)(const struct aug_var*, const char*);
    void (*setfield_)(const struct aug_var*, const char*, const char*);
    void (*setcsize_)(const struct aug_var*, unsigned);
    void (*cdata_)(const struct aug_var*, const void*, unsigned);
    void (*end_)(const struct aug_var*, int);
};

typedef struct aug_httpparser_* aug_httpparser_t;

AUGNET_API aug_httpparser_t
aug_createhttpparser(size_t size, const struct aug_httphandlers* handlers,
                     const struct aug_var* arg);

AUGNET_API int
aug_freehttpparser(aug_httpparser_t parser);

AUGNET_API int
aug_parsehttp(aug_httpparser_t parser, const char*, size_t size);

AUGNET_API int
aug_endhttp(aug_httpparser_t parser);

#endif /* AUGNET_HTTP_H */
