/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_HTTP_H
#define AUGNET_HTTP_H

#include "augnet/config.h"
#include "augnet/types.h"

typedef struct aug_httpparser_* aug_httpparser_t;

AUGNET_API aug_httpparser_t
aug_createhttpparser(size_t size, const struct aug_handlers* handlers,
                     void* arg);

AUGNET_API int
aug_freehttpparser(aug_httpparser_t parser);

AUGNET_API int
aug_parsehttp(aug_httpparser_t parser, const char* buf, size_t size);

AUGNET_API int
aug_endhttp(aug_httpparser_t parser);

#endif /* AUGNET_HTTP_H */
