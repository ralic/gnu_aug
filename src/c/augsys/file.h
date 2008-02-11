/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Sun Feb 10 20:50:30 GMT Standard Time 2008 */

#ifndef AUGSYS_FILE_H
#define AUGSYS_FILE_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */
#include "augsys/config.h"
#include "augctx/ctx.h"
#if !defined(_WIN32)
typedef int aug_fd;
#else /* _WIN32 */
typedef void* aug_fd;
#endif /* _WIN32 */
#define AUG_BADFD ((aug_fd)-1)

/**
 * @defgroup aug_file aug_file
 *
 * @ingroup Object
 *
 * @{
 */

AUG_INTERFACE(aug_file);

struct aug_filevtbl {
    AUG_VTBL(aug_file);
    int (*close_)(aug_file*);
    int (*setnonblock_)(aug_file*, int);
    aug_fd (*getfd_)(aug_file*);
};

#define aug_close(this_) \
    (this_)->vtbl_->close_(this_)

#define aug_setnonblock(this_, on) \
    (this_)->vtbl_->setnonblock_(this_, on)

#define aug_getfd(this_) \
    (this_)->vtbl_->getfd_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_file> {
        typedef aug_filevtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_fileid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_file> fileref;

    inline int
    close(aug::obref<aug_file> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->close_(this_.get());
    }

    inline int
    setnonblock(aug::obref<aug_file> this_, int on) AUG_NOTHROW
    {
        return this_.get()->vtbl_->setnonblock_(this_.get(), on);
    }

    inline aug_fd
    getfd(aug::obref<aug_file> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getfd_(this_.get());
    }

    template <typename T>
    class file {

        aug_file file_;

        file(const file&);

        file&
        operator =(const file&);

        static void*
        cast_(aug_file* this_, const char* id) AUG_NOTHROW
        {
            return aug::incget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_file* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_file* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static int
        close_(aug_file* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->close_();
        }
        static int
        setnonblock_(aug_file* this_, int on) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->setnonblock_(on);
        }
        static aug_fd
        getfd_(aug_file* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getfd_();
        }
        static const aug_filevtbl*
        vtbl()
        {
            static const aug_filevtbl local = {
                cast_,
                retain_,
                release_,
                close_,
                setnonblock_,
                getfd_
            };
            return &local;
        }
    public:
        explicit
        file(T* impl = 0)
        {
            this->file_.vtbl_ = this->vtbl();
            this->file_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->file_.impl_ = impl;
        }
        aug_file*
        get() AUG_NOTHROW
        {
            return &this->file_;
        }
        operator aug::obref<aug_file>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_file {
        file<basic_file<T> > file_;
        T impl_;
        int refs_;
        explicit
        basic_file(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->file_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_file>(id))
                return aug::object_retain<aug_object>(this->file_);
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
        int
        close_() AUG_NOTHROW
        {
            return this->impl_.close_();
        }
        int
        setnonblock_(int on) AUG_NOTHROW
        {
            return this->impl_.setnonblock_(on);
        }
        aug_fd
        getfd_() AUG_NOTHROW
        {
            return this->impl_.getfd_();
        }
        static aug::smartob<aug_file>
        create(const T& impl = T())
        {
            basic_file* ptr(new basic_file(impl));
            return aug::object_attach<aug_file>(ptr->file_);
        }
    };

    template <typename T>
    class scoped_file {
        file<scoped_file<T> > file_;
        T impl_;
    public:
        explicit
        scoped_file(const T& impl = T())
            : impl_(impl)
        {
            this->file_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_file>(id))
                return aug::object_retain<aug_object>(this->file_);
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
        int
        close_() AUG_NOTHROW
        {
            return this->impl_.close_();
        }
        int
        setnonblock_(int on) AUG_NOTHROW
        {
            return this->impl_.setnonblock_(on);
        }
        aug_fd
        getfd_() AUG_NOTHROW
        {
            return this->impl_.getfd_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->file_).get();
        }
        aug_file*
        get() AUG_NOTHROW
        {
            return this->file_.get();
        }
        operator aug::obref<aug_file>() AUG_NOTHROW
        {
            return this->file_;
        }
    };
}
#endif /* __cplusplus */
AUGSYS_API aug_file*
aug_createfile(aug_ctx* ctx, const char* path, int flags, ...);

#endif /* AUGSYS_FILE_H */
