/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/mpool.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#define DEBUG 1
#define ONLY_MSPACES 1
#include "malloc.c"

#include <assert.h>
#include <string.h>

static void*
ccast_(aug_mpool* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_mpoolid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
cretain_(aug_mpool* obj)
{
}

static void
crelease_(aug_mpool* obj)
{
}

static void*
cmalloc_(aug_mpool* obj, size_t size)
{
    return malloc(size);
}

static void
cfree_(aug_mpool* obj, void* ptr)
{
    free(ptr);
}

static void*
crealloc_(aug_mpool* obj, void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void*
ccalloc_(aug_mpool* obj, size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

static const struct aug_mpoolvtbl cvtbl_ = {
    ccast_,
    cretain_,
    crelease_,
    cmalloc_,
    cfree_,
    crealloc_,
    ccalloc_
};

static aug_mpool mpool_ = { &cvtbl_, NULL };

AUGCTX_API aug_mpool*
aug_createcmalloc(void)
{
    return &mpool_;
}

struct dlimpl_ {
    aug_mpool mpool_;
    int refs_;
    mspace mspace_;
};

static void*
dlcast_(aug_mpool* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_mpoolid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
dlretain_(aug_mpool* obj)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
dlrelease_(aug_mpool* obj)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        mspace msp = impl->mspace_;
        mspace_free(msp, impl);
#if DEBUG
        fprintf(stderr, "mspace before destroy...\n");
        mspace_malloc_stats(msp);
#endif /* DEBUG */
        destroy_mspace(msp);
    }
}

static void*
dlmalloc_(aug_mpool* obj, size_t size)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    return mspace_malloc(impl->mspace_, size);
}

static void
dlfree_(aug_mpool* obj, void* ptr)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    mspace_free(impl->mspace_, ptr);
}

static void*
dlrealloc_(aug_mpool* obj, void* ptr, size_t size)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    return mspace_realloc(impl->mspace_, ptr, size);
}

static void*
dlcalloc_(aug_mpool* obj, size_t nmemb, size_t size)
{
    struct dlimpl_* impl = AUG_PODIMPL(struct dlimpl_, mpool_, obj);
    return mspace_calloc(impl->mspace_, nmemb, size);
}

static const struct aug_mpoolvtbl dlvtbl_ = {
    dlcast_,
    dlretain_,
    dlrelease_,
    dlmalloc_,
    dlfree_,
    dlrealloc_,
    dlcalloc_
};

AUGCTX_API aug_mpool*
aug_createdlmalloc(void)
{
    mspace msp;
    struct dlimpl_* impl;

    if (!(msp = create_mspace(0, 0)))
        return NULL;

#if DEBUG
    fprintf(stderr, "mspace after create...\n");
    mspace_malloc_stats(msp);
#endif /* DEBUG */

    if (!(impl = mspace_malloc(msp, sizeof(struct dlimpl_)))) {
        destroy_mspace(msp);
        return NULL;
    }

    impl->mpool_.vtbl_ = &dlvtbl_;
    impl->mpool_.impl_ = NULL;

    impl->refs_ = 1;
    impl->mspace_ = msp;

    return &impl->mpool_;
}
