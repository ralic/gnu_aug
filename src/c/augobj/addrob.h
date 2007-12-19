/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Wed Dec 19 02:37:18 GMT Standard Time 2007 */

#ifndef AUG_ADDROB_H
#define AUG_ADDROB_H

#include "augobj.h"

#if defined(__cplusplus)

#include "augobjpp.hpp"

# if !defined(AUG_NOTHROW)
#  define AUG_NOTHROW
# endif /* !AUG_NOTHROW */

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

AUG_OBJECTDECL(aug_addrob);
struct aug_addrobvtbl {
    AUG_OBJECT(aug_addrob);
    void* (*get_)(aug_addrob*);
};

#define aug_getaddrob(this_) \
    ((aug_addrob*)this_)->vtbl_->get_(this_)

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_addrob> {
        typedef aug_addrobvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_addrobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_addrob> addrobref;

    inline void*
    getaddrob(aug::obref<aug_addrob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->get_(this_.get());
    }

    template <typename T>
    class addrob {

        aug_addrob addrob_;

        addrob(const addrob&);

        addrob&
        operator =(const addrob&);

        static void*
        cast_(aug_addrob* this_, const char* id) AUG_NOTHROW
        {
            return aug::incget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static int
        retain_(aug_addrob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->retain_();
        }
        static int
        release_(aug_addrob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->release_();
        }
        static void*
        get_(aug_addrob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getaddrob_();
        }
        static const aug_addrobvtbl*
        vtbl()
        {
            static const aug_addrobvtbl local = {
                cast_,
                retain_,
                release_,
                get_
            };
            return &local;
        }
    public:
        explicit
        addrob(T* impl = 0)
        {
            this->addrob_.vtbl_ = this->vtbl();
            this->addrob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->addrob_.impl_ = impl;
        }
        aug_addrob*
        get() AUG_NOTHROW
        {
            return &this->addrob_;
        }
        operator aug::obref<aug_addrob>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_addrob {
        addrob<basic_addrob<T> > addrob_;
        T impl_;
        int refs_;
        explicit
        basic_addrob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->addrob_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_addrob>(id))
                return aug::object_retain<aug_object>(this->addrob_);
            return null;
        }
        int
        retain_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            ++this->refs_;
            return 0;
        }
        int
        release_() AUG_NOTHROW
        {
            assert(0 < this->refs_);
            if (0 == --this->refs_)
                delete this;
            return 0;
        }
        void*
        getaddrob_() AUG_NOTHROW
        {
            return this->impl_.getaddrob_();
        }
        static aug::smartob<aug_addrob>
        create(const T& impl = T())
        {
            basic_addrob* ptr(new basic_addrob(impl));
            return aug::object_attach<aug_addrob>(ptr->addrob_);
        }
    };

    template <typename T>
    class scoped_addrob {
        addrob<scoped_addrob<T> > addrob_;
        T impl_;
    public:
        explicit
        scoped_addrob(const T& impl = T())
            : impl_(impl)
        {
            this->addrob_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_addrob>(id))
                return aug::object_retain<aug_object>(this->addrob_);
            return null;
        }
        int
        retain_() AUG_NOTHROW
        {
            return 0;
        }
        int
        release_() AUG_NOTHROW
        {
            return 0;
        }
        void*
        getaddrob_() AUG_NOTHROW
        {
            return this->impl_.getaddrob_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->addrob_).get();
        }
        aug_addrob*
        get() AUG_NOTHROW
        {
            return this->addrob_.get();
        }
        operator aug::obref<aug_addrob>() AUG_NOTHROW
        {
            return this->addrob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_ADDROB_H */