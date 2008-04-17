/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Thu Apr 17 07:19:34 GMT Daylight Time 2008 */

#ifndef AUGCTX_MPOOL_H
#define AUGCTX_MPOOL_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */
#include "augctx/config.h"

/**
 * @defgroup aug_mpool aug_mpool
 *
 * @ingroup Object
 *
 * @{
 */

AUG_INTERFACE(aug_mpool);

struct aug_mpoolvtbl {
    AUG_VTBL(aug_mpool);
    void* (*malloc_)(aug_mpool*, size_t);
    void (*free_)(aug_mpool*, void*);
    void* (*realloc_)(aug_mpool*, void*, size_t);
    void* (*calloc_)(aug_mpool*, size_t, size_t);
};

#define aug_malloc(this_, size) \
    (this_)->vtbl_->malloc_(this_, size)

#define aug_free(this_, ptr) \
    (this_)->vtbl_->free_(this_, ptr)

#define aug_realloc(this_, ptr, size) \
    (this_)->vtbl_->realloc_(this_, ptr, size)

#define aug_calloc(this_, nmemb, size) \
    (this_)->vtbl_->calloc_(this_, nmemb, size)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_mpool> {
        typedef aug_mpoolvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_mpoolid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_mpool> mpoolref;

    inline void*
    malloc(aug::obref<aug_mpool> this_, size_t size) AUG_NOTHROW
    {
        return this_.get()->vtbl_->malloc_(this_.get(), size);
    }

    inline void
    free(aug::obref<aug_mpool> this_, void* ptr) AUG_NOTHROW
    {
        this_.get()->vtbl_->free_(this_.get(), ptr);
    }

    inline void*
    realloc(aug::obref<aug_mpool> this_, void* ptr, size_t size) AUG_NOTHROW
    {
        return this_.get()->vtbl_->realloc_(this_.get(), ptr, size);
    }

    inline void*
    calloc(aug::obref<aug_mpool> this_, size_t nmemb, size_t size) AUG_NOTHROW
    {
        return this_.get()->vtbl_->calloc_(this_.get(), nmemb, size);
    }

    template <typename T>
    class mpool {

        aug_mpool mpool_;

        mpool(const mpool&);

        mpool&
        operator =(const mpool&);

        static void*
        cast_(aug_mpool* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_mpool* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_mpool* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static void*
        malloc_(aug_mpool* this_, size_t size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->malloc_(size);
        }
        static void
        free_(aug_mpool* this_, void* ptr) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->free_(ptr);
        }
        static void*
        realloc_(aug_mpool* this_, void* ptr, size_t size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->realloc_(ptr, size);
        }
        static void*
        calloc_(aug_mpool* this_, size_t nmemb, size_t size) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->calloc_(nmemb, size);
        }
        static const aug_mpoolvtbl*
        vtbl()
        {
            static const aug_mpoolvtbl local = {
                cast_,
                retain_,
                release_,
                malloc_,
                free_,
                realloc_,
                calloc_
            };
            return &local;
        }
    public:
        explicit
        mpool(T* impl = 0)
        {
            this->mpool_.vtbl_ = this->vtbl();
            this->mpool_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->mpool_.impl_ = impl;
        }
        aug_mpool*
        get() AUG_NOTHROW
        {
            return &this->mpool_;
        }
        operator aug::obref<aug_mpool>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_mpool {
        mpool<basic_mpool<T> > mpool_;
        T impl_;
        int refs_;
        explicit
        basic_mpool(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->mpool_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_mpool>(id))
                return aug::object_retain<aug_object>(this->mpool_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            ++this->refs_;
        }
        void
        release_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            if (0 == --this->refs_)
                delete this;
        }
        void*
        malloc_(size_t size) AUG_NOTHROW
        {
            return this->impl_.malloc_(size);
        }
        void
        free_(void* ptr) AUG_NOTHROW
        {
            this->impl_.free_(ptr);
        }
        void*
        realloc_(void* ptr, size_t size) AUG_NOTHROW
        {
            return this->impl_.realloc_(ptr, size);
        }
        void*
        calloc_(size_t nmemb, size_t size) AUG_NOTHROW
        {
            return this->impl_.calloc_(nmemb, size);
        }
        static aug::smartob<aug_mpool>
        create(const T& impl = T())
        {
            basic_mpool* ptr(new basic_mpool(impl));
            return aug::object_attach<aug_mpool>(ptr->mpool_);
        }
    };

    template <typename T>
    class scoped_mpool {
        mpool<scoped_mpool<T> > mpool_;
        T impl_;
    public:
        explicit
        scoped_mpool(const T& impl = T())
            : impl_(impl)
        {
            this->mpool_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_mpool>(id))
                return aug::object_retain<aug_object>(this->mpool_);
            return null;
        }
        void
        retain_() AUG_NOTHROW
        {
        }
        void
        release_() AUG_NOTHROW
        {
        }
        void*
        malloc_(size_t size) AUG_NOTHROW
        {
            return this->impl_.malloc_(size);
        }
        void
        free_(void* ptr) AUG_NOTHROW
        {
            this->impl_.free_(ptr);
        }
        void*
        realloc_(void* ptr, size_t size) AUG_NOTHROW
        {
            return this->impl_.realloc_(ptr, size);
        }
        void*
        calloc_(size_t nmemb, size_t size) AUG_NOTHROW
        {
            return this->impl_.calloc_(nmemb, size);
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->mpool_).get();
        }
        aug_mpool*
        get() AUG_NOTHROW
        {
            return this->mpool_.get();
        }
        operator aug::obref<aug_mpool>() AUG_NOTHROW
        {
            return this->mpool_;
        }
    };
}
#endif /* __cplusplus */
AUGCTX_API aug_mpool*
aug_createcmalloc(void);

AUGCTX_API aug_mpool*
aug_createdlmalloc(void);

#endif /* AUGCTX_MPOOL_H */
