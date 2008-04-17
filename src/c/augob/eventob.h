/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Thu Apr 17 07:20:01 GMT Daylight Time 2008 */

#ifndef AUGOB_EVENTOB_H
#define AUGOB_EVENTOB_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */

/**
 * @defgroup aug_eventob aug_eventob
 *
 * @ingroup Object
 *
 * @{
 */

AUG_INTERFACE(aug_eventob);

struct aug_eventobvtbl {
    AUG_VTBL(aug_eventob);
    void (*setuser_)(aug_eventob*, aug_object*);
    const char* (*from_)(aug_eventob*);
    const char* (*to_)(aug_eventob*);
    const char* (*type_)(aug_eventob*);
    aug_object* (*user_)(aug_eventob*);
};

#define aug_seteventobuser(this_, user) \
    (this_)->vtbl_->setuser_(this_, user)

#define aug_eventobfrom(this_) \
    (this_)->vtbl_->from_(this_)

#define aug_eventobto(this_) \
    (this_)->vtbl_->to_(this_)

#define aug_eventobtype(this_) \
    (this_)->vtbl_->type_(this_)

#define aug_eventobuser(this_) \
    (this_)->vtbl_->user_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_eventob> {
        typedef aug_eventobvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_eventobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_eventob> eventobref;

    inline void
    seteventobuser(aug::obref<aug_eventob> this_, aug::obref<aug_object> user) AUG_NOTHROW
    {
        this_.get()->vtbl_->setuser_(this_.get(), user.get());
    }

    inline const char*
    eventobfrom(aug::obref<aug_eventob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->from_(this_.get());
    }

    inline const char*
    eventobto(aug::obref<aug_eventob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->to_(this_.get());
    }

    inline const char*
    eventobtype(aug::obref<aug_eventob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->type_(this_.get());
    }

    inline aug::smartob<aug_object>
    eventobuser(aug::obref<aug_eventob> this_) AUG_NOTHROW
    {
        return aug::object_attach<aug_object>(this_.get()->vtbl_->user_(this_.get()));
    }

    template <typename T>
    class eventob {

        aug_eventob eventob_;

        eventob(const eventob&);

        eventob&
        operator =(const eventob&);

        static void*
        cast_(aug_eventob* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_eventob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_eventob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static void
        setuser_(aug_eventob* this_, aug_object* user) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->seteventobuser_(user);
        }
        static const char*
        from_(aug_eventob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->eventobfrom_();
        }
        static const char*
        to_(aug_eventob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->eventobto_();
        }
        static const char*
        type_(aug_eventob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->eventobtype_();
        }
        static aug_object*
        user_(aug_eventob* this_) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->eventobuser_());
        }
        static const aug_eventobvtbl*
        vtbl()
        {
            static const aug_eventobvtbl local = {
                cast_,
                retain_,
                release_,
                setuser_,
                from_,
                to_,
                type_,
                user_
            };
            return &local;
        }
    public:
        explicit
        eventob(T* impl = 0)
        {
            this->eventob_.vtbl_ = this->vtbl();
            this->eventob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->eventob_.impl_ = impl;
        }
        aug_eventob*
        get() AUG_NOTHROW
        {
            return &this->eventob_;
        }
        operator aug::obref<aug_eventob>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_eventob {
        eventob<basic_eventob<T> > eventob_;
        T impl_;
        int refs_;
        explicit
        basic_eventob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->eventob_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_eventob>(id))
                return aug::object_retain<aug_object>(this->eventob_);
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
        void
        seteventobuser_(aug::obref<aug_object> user) AUG_NOTHROW
        {
            this->impl_.seteventobuser_(user);
        }
        const char*
        eventobfrom_() AUG_NOTHROW
        {
            return this->impl_.eventobfrom_();
        }
        const char*
        eventobto_() AUG_NOTHROW
        {
            return this->impl_.eventobto_();
        }
        const char*
        eventobtype_() AUG_NOTHROW
        {
            return this->impl_.eventobtype_();
        }
        aug::smartob<aug_object>
        eventobuser_() AUG_NOTHROW
        {
            return this->impl_.eventobuser_();
        }
        static aug::smartob<aug_eventob>
        create(const T& impl = T())
        {
            basic_eventob* ptr(new basic_eventob(impl));
            return aug::object_attach<aug_eventob>(ptr->eventob_);
        }
    };

    template <typename T>
    class scoped_eventob {
        eventob<scoped_eventob<T> > eventob_;
        T impl_;
    public:
        explicit
        scoped_eventob(const T& impl = T())
            : impl_(impl)
        {
            this->eventob_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_eventob>(id))
                return aug::object_retain<aug_object>(this->eventob_);
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
        void
        seteventobuser_(aug::obref<aug_object> user) AUG_NOTHROW
        {
            this->impl_.seteventobuser_(user);
        }
        const char*
        eventobfrom_() AUG_NOTHROW
        {
            return this->impl_.eventobfrom_();
        }
        const char*
        eventobto_() AUG_NOTHROW
        {
            return this->impl_.eventobto_();
        }
        const char*
        eventobtype_() AUG_NOTHROW
        {
            return this->impl_.eventobtype_();
        }
        aug::smartob<aug_object>
        eventobuser_() AUG_NOTHROW
        {
            return this->impl_.eventobuser_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->eventob_).get();
        }
        aug_eventob*
        get() AUG_NOTHROW
        {
            return this->eventob_.get();
        }
        operator aug::obref<aug_eventob>() AUG_NOTHROW
        {
            return this->eventob_;
        }
    };
}
#endif /* __cplusplus */

#endif /* AUGOB_EVENTOB_H */
