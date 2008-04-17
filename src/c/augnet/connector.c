/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/connector.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/inet.h"    /* aug_established() */

#include "augsys/base.h"    /* aug_getosfd() */
#include "augsys/unistd.h"  /* aug_close() */
#include "augsys/utility.h" /* aug_setnonblock() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/string.h"  /* bzero() */

#include <assert.h>
#include <stdlib.h>         /* malloc() */

struct aug_connector_ {
    struct addrinfo* res_, *save_;
    int fd_;
};

static int
getsockerr_(int s, int* err)
{
    socklen_t len = sizeof(*err);
    if (-1 == aug_getsockopt(s, SOL_SOCKET, SO_ERROR, err, &len))
        return -1;

    return 0;
}

AUGNET_API aug_connector_t
aug_createconnector(const char* host, const char* serv)
{
    aug_connector_t ctor;
    struct addrinfo hints, * res;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return NULL;

    if (!(ctor  = malloc(sizeof(struct aug_connector_)))) {
        aug_destroyaddrinfo(res);
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    ctor->res_ = ctor->save_ = res;
    ctor->fd_ = -1;
    return ctor;
}

AUGNET_API int
aug_destroyconnector(aug_connector_t ctor)
{
    if (-1 != ctor->fd_)
        aug_fclose(aug_getosfd(ctor->fd_));

    aug_destroyaddrinfo(ctor->save_);
    free(ctor);
    return 0;
}

AUGNET_API int
aug_tryconnect(aug_connector_t ctor, struct aug_endpoint* ep, int* est)
{
    int fd = ctor->fd_;
    ctor->fd_ = -1;

    /* Handle case where user has called after all connection attempts have
       failed. */

    if (!ctor->res_) {
        assert(-1 == fd);
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("address-list exhausted"));
        return -1;
    }

    if (-1 != fd) {

        /* Check status of pending connection. */

        switch (aug_established(fd)) {
        case -1:

            /* Error testing for establishment. */

            aug_fclose(aug_getosfd(fd));
            return -1;

        case 0:

            /* Established. */

            aug_getendpoint(ctor->res_, ep);
            goto done;

        case AUG_RETNONE:

            /* Not established. */

            if ((ctor->res_ = ctor->res_->ai_next)) {

                /* Try next address. */

                if (-1 == aug_fclose(aug_getosfd(fd)))
                    return -1;
                break;
            }

            /* No more addresses: set error to connection failure reason. */

            {
                int err = ECONNREFUSED;
                getsockerr_(fd, &err);
                aug_fclose(aug_getosfd(fd)); /* May set errno */
                aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, err);
            }
            return -1;
        }
    }

    do {

        /* Create socket for next address. */

        fd = aug_socket(ctor->res_->ai_family, ctor->res_->ai_socktype,
                        ctor->res_->ai_protocol);
        if (-1 == fd)
            continue; /* Ignore this one. */

        if (-1 == aug_fsetnonblock(aug_getosfd(fd), 1)) {
            aug_fclose(aug_getosfd(fd));
            return -1;
        }

        aug_getendpoint(ctor->res_, ep);

        if (0 == aug_connect(fd, ep)) {

            /* Immediate establishment. */

            goto done;
        }

        if (EINPROGRESS == aug_errno() || EWOULDBLOCK == aug_errno()) {

            /* Connection pending. */

            *est = 0;
            return ctor->fd_ = fd;
        }

        /* Failed for other reason. */

        if (-1 == aug_fclose(aug_getosfd(fd))) /* Ignore this one. */
            return -1;

    } while ((ctor->res_ = ctor->res_->ai_next));

    /* Error set from last aug_connect() attempt. */

    return -1;

 done:

    /* Set back to blocking. */

    if (-1 == aug_fsetnonblock(aug_getosfd(fd), 0)) {
        aug_fclose(aug_getosfd(fd));
        return -1;
    }

    *est = 1;
    return fd;
}
