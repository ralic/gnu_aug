/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/writer.h"

static const char rcsid[] = "$Id$";

#include "augsys/lock.h"
#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/uio.h"
#include "augutil/list.h"
#include "augutil/var.h"

#if !defined(_WIN32)
# if HAVE_ALLOCA_H
#  include <alloca.h>
# endif /* HAVE_ALLOCA_H */
#else /* _WIN32 */
# include <malloc.h>
#endif /* _WIN32 */

#include <stdlib.h> /* malloc() */

struct aug_buf {
    AUG_ENTRY(aug_buf);
    struct aug_var var_;
};

AUG_HEAD(aug_bufs, aug_buf);

static struct aug_bufs free_ = AUG_HEAD_INITIALIZER(free_);
AUG_ALLOCATOR(allocate_, &free_, aug_buf, 64)

struct aug_writer_ {
    struct aug_bufs bufs_;
    size_t part_;
    unsigned size_;
};

static struct aug_buf*
createbuf_(const struct aug_var* var)
{
    struct aug_buf* buf;

    aug_lock();
    if (!(buf = allocate_())) {
        aug_unlock();
        return NULL;
    }
    aug_unlock();

    aug_setvar(&buf->var_, var);
    return buf;
}

static void
destroybufs_(struct aug_bufs* bufs)
{
    if (!AUG_EMPTY(bufs)) {
        aug_lock();
        AUG_CONCAT(&free_, bufs);
        aug_unlock();
    }
}

static void
destroybuf_(struct aug_buf* buf)
{
    aug_lock();
    AUG_INSERT_TAIL(&free_, buf);
    aug_unlock();
}

static void
popbufs_(aug_writer_t writer, const struct iovec* iov, size_t num)
{
    while ((size_t)iov->iov_len <= num) {

        struct aug_buf* it = AUG_FIRST(&writer->bufs_);
        AUG_REMOVE_HEAD(&writer->bufs_);

        aug_destroyvar(&it->var_);
        destroybuf_(it);

        --writer->size_;
        num -= (iov++)->iov_len;
    }

    /* Store offset into current for next write. */

    writer->part_ = num;
}

AUGNET_API aug_writer_t
aug_createwriter(void)
{
    aug_writer_t writer = malloc(sizeof(struct aug_writer_));
    if (!writer) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    AUG_INIT(&writer->bufs_);
    writer->part_ = 0;
    writer->size_ 
= 0;
    return writer;
}

AUGNET_API int
aug_destroywriter(aug_writer_t writer)
{
    struct aug_buf* it;
    AUG_FOREACH(it, &writer->bufs_) {
        aug_destroyvar(&it->var_);
    }

    /* Destroy in single batch to avoid multiple calls to aug_lock(). */

    destroybufs_(&writer->bufs_);
    return 0;
}

AUGNET_API int
aug_appendwriter(aug_writer_t writer, const struct aug_var* var)
{
    struct aug_buf* buf = createbuf_(var);
    if (!buf)
        return -1;

    AUG_INSERT_TAIL(&writer->bufs_, buf);
    ++writer->size_;
    return 0;
}

AUGNET_API int
aug_emptywriter(aug_writer_t writer)
{
    return AUG_EMPTY(&writer->bufs_);
}

AUGNET_API ssize_t
aug_writesome(aug_writer_t writer, int fd)
{
    ssize_t ret;
    struct iovec* iov;
    struct aug_buf* it;
    unsigned i, size;
    size_t len;

    /* As a precaution, limit use of stack space. */

    if (0 == (size = AUG_MIN(writer->size_, 64)))
        return 0;

    iov = alloca(sizeof(struct iovec) * size);

    /* Map each buffer to an entry in the iov table. */

    i = 0;
    AUG_FOREACH(it, &writer->bufs_) {

        if (!(iov[i].iov_base = (void*)aug_varbuf(&it->var_, &len)))
            return -1;

        iov[i].iov_len = (int)len;
        if (++i == size)
            break; /* Exhausted vector size. */
    }

    /* Add offset to first entry. */

    iov->iov_base = (char*)iov->iov_base + writer->part_;
    iov->iov_len -= (int)writer->part_;

    if (-1 != (ret = aug_writev(fd, iov, size))) {

        /* Pop any completed buffers from queue. */

        popbufs_(writer, iov, ret);
    }

    return ret;
}