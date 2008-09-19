/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/unistd.h" /* aug_fsize() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/mman.h>

#define FLAGMASK_ (AUG_MMAPRD | AUG_MMAPWR)

typedef struct impl_ {
    struct aug_mmap mmap_;
    aug_mpool* mpool_;
    aug_fd fd_, prot_;
    size_t size_;
}* impl_t;

static aug_result
toprot_(int* to, int from)
{
    int prot = 0;
    if (from & ~FLAGMASK_)
        goto fail;

    if (AUG_MMAPRD == (from & AUG_MMAPRD))
        prot |= PROT_READ;

    if (AUG_MMAPWR == (from & AUG_MMAPWR))
        prot |= PROT_WRITE;

    if (!prot)
        prot = PROT_NONE;

    *to = prot;
    return AUG_SUCCESS;

 fail:
    aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                   AUG_MSG("invalid protection flags '%d'"), (int)from);
    return AUG_FAILERROR;
}

static aug_result
verify_(size_t size, size_t offset, size_t len)
{
    /* An empty file cannot be mapped, in addition, the size to be mapped
       should not be greater than the file size. */

    if (!size || size < (offset + len)) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid file map size '%d'"), (int)size);
        return AUG_FAILERROR;
    }

    /* The offset if specified must occur on an allocation size boundary. */

    if (offset && (offset % aug_granularity())) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid file map offset '%d'"), (int)offset);
        return AUG_FAILERROR;
    }

    return AUG_SUCCESS;
}

static aug_result
createmmap_(impl_t impl, size_t offset, size_t len)
{
    void* addr;

    if (!len)
        len = impl->size_ - offset;

    if (MAP_FAILED == (addr = mmap(NULL, len, impl->prot_, MAP_SHARED,
                                   impl->fd_, (off_t)offset)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    impl->mmap_.addr_ = addr;
    impl->mmap_.len_ = len;
    return AUG_SUCCESS;
}

static aug_result
destroymmap_(impl_t impl)
{
    if (impl->mmap_.addr_
        && -1 == munmap(impl->mmap_.addr_, impl->mmap_.len_))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

AUGSYS_API void
aug_destroymmap(struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    aug_mpool* mpool = impl->mpool_;
    destroymmap_(impl);
    aug_freemem(mpool, impl);
    aug_release(mpool);
}

AUGSYS_API struct aug_mmap*
aug_createmmap(aug_mpool* mpool, aug_fd fd, size_t offset, size_t len,
               int flags)
{
    impl_t impl;
    int prot;
    size_t size;

    if (AUG_ISFAIL(toprot_(&prot, flags)))
        return NULL;

    if (AUG_ISFAIL(aug_fsize(fd, &size)))
        return NULL;

    if (AUG_ISFAIL(verify_(size, offset, len)))
        return NULL;

    if (!(impl = aug_allocmem(mpool, sizeof(struct impl_))))
        return NULL;

    impl->mmap_.addr_ = NULL;
    impl->mmap_.len_ = 0;
    impl->mpool_ = mpool;
    impl->fd_ = fd;
    impl->prot_ = prot;
    impl->size_ = size;

    if (AUG_ISFAIL(createmmap_(impl, offset, len))) {
        aug_freemem(mpool, impl);
        return NULL;
    }
    aug_retain(mpool);
    return (struct aug_mmap*)impl;
}

AUGSYS_API aug_result
aug_remmap(struct aug_mmap* mm, size_t offset, size_t len)
{
    impl_t impl = (impl_t)mm;
    void* addr = impl->mmap_.addr_;
    aug_result result;

    impl->mmap_.addr_ = NULL;

    if (addr && -1 == munmap(addr, impl->mmap_.len_))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    if (impl->size_ < (offset + len)) {

        if (AUG_ISFAIL(result = aug_fsize(impl->fd_, &impl->size_)))
            return result;
    }

    if (AUG_ISFAIL(result = verify_(impl->size_, offset, len)))
        return result;

    return createmmap_(impl, offset, len);
}

AUGSYS_API aug_result
aug_syncmmap(const struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    if (-1 == msync(impl->mmap_.addr_, impl->mmap_.len_, MS_SYNC))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    return AUG_SUCCESS;
}

AUGSYS_API size_t
aug_mmapsize(const struct aug_mmap* mm)
{
    impl_t impl = (impl_t)mm;
    return impl->size_;
}

AUGSYS_API unsigned
aug_granularity(void)
{
    /* Not documented to return an error. */

    return getpagesize();
}

AUGSYS_API unsigned
aug_pagesize(void)
{
    /* Not documented to return an error. */

    return getpagesize();
}
