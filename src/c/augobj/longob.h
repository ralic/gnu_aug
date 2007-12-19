/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Wed Dec 19 02:37:19 GMT Standard Time 2007 */

#ifndef AUG_LONGOB_H
#define AUG_LONGOB_H

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

AUG_OBJECTDECL(aug_longob);
struct aug_longobvtbl {
    AUG_OBJECT(aug_longob);
    long (*get_)(aug_longob*);
};

#define aug_getlongob(this_) \
    ((aug_longob*)this_)->vtbl_->get_(this_)

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_longob> {
        typedef aug_longobvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_longobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_longob> longobref;

    inline long
    getlongob(aug::obref<aug_longob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->get_(this_.get());
    }

    template <typename T>
    class longob {

        aug_longob longob_;

        longob(const longob&);

        longob&
        operator =(const longob&);

        static void*
        cast_(aug_longob* this_, const char* id) AUG_NOTHROW
        {
            return aug::incget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static int
        retain_(aug_longob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->retain_();
        }
        static int
        release_(aug_longob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->release_();
        }
        static long
        get_(aug_longob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getlongob_();
        }
        static const aug_longobvtbl*
        vtbl()
        {
            static const aug_longobvtbl local = {
                cast_,
                retain_,
                release_,
                get_
            };
            return &local;
        }
    public:
        explicit
        longob(T* impl = 0)
        {
            this->longob_.vtbl_ = this->vtbl();
            this->longob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->longob_.impl_ = impl;
        }
        aug_longob*
        get() AUG_NOTHROW
        {
            return &this->longob_;
        }
        operator aug::obref<aug_longob>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_longob {
        longob<basic_longob<T> > longob_;
        T impl_;
        int refs_;
        explicit
        basic_longob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->longob_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_longob>(id))
                return aug::object_retain<aug_object>(this->longob_);
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
        long
        getlongob_() AUG_NOTHROW
        {
            return this->impl_.getlongob_();
        }
        static aug::smartob<aug_longob>
        create(const T& impl = T())
        {
            basic_longob* ptr(new basic_longob(impl));
            return aug::object_attach<aug_longob>(ptr->longob_);
        }
    };

    template <typename T>
    class scoped_longob {
        longob<scoped_longob<T> > longob_;
        T impl_;
    public:
        explicit
        scoped_longob(const T& impl = T())
            : impl_(impl)
        {
            this->longob_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_longob>(id))
                return aug::object_retain<aug_object>(this->longob_);
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
        long
        getlongob_() AUG_NOTHROW
        {
            return this->impl_.getlongob_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->longob_).get();
        }
        aug_longob*
        get() AUG_NOTHROW
        {
            return this->longob_.get();
        }
        operator aug::obref<aug_longob>() AUG_NOTHROW
        {
            return this->longob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUG_LONGOB_H */