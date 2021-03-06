/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

#define HOST_ "127.0.0.1"
#define SERV_ "5000"

int
main(int argc, char* argv[])
{
    struct addrinfo hints, * res, * save;

    if (!aug_autotlx())
        return 1;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (aug_getaddrinfo(HOST_, SERV_, &hints, &res) < 0) {
        aug_perrinfo(aug_tlx, "aug_getaddrinfo() failed", NULL);
        return 1;
    }

    save = res;
    do {
        struct aug_endpoint ep;
        char buf[AUG_MAXHOSTSERVLEN + 1];

        aug_getendpoint(res, &ep);
        if (!aug_endpointntop(&ep, buf, sizeof(buf))) {
            aug_perrinfo(aug_tlx, "aug_endpointntop() failed", NULL);
            goto fail;
        }

        if (0 != strcmp(buf, "127.0.0.1:5000")) {
            fprintf(stderr, "unexpected address [%s]\n", buf);
            goto fail;
        }

    } while ((res = res->ai_next));

    aug_destroyaddrinfo(save);
    return 0;

 fail:
    aug_destroyaddrinfo(save);
    return 1;
}
