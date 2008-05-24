/* -*- c++ -*- */
/* Automatically generated by augidl */
/* at Fri May 23 08:15:29 GMT Daylight Time 2008 */

#ifndef AUGOB_CHANNELOB_H
#define AUGOB_CHANNELOB_H

#include "augabi.h"

#if defined(__cplusplus)

#include "augabipp.hpp"

namespace aug {
    template <typename T>
    struct object_traits;
}

#endif /* __cplusplus */
#include "augob/streamob.h"
typedef aug_bool (*aug_channelcb_t)(aug_object*, unsigned, aug_streamob*,
      unsigned short);

/**
 * @defgroup aug_channelob aug_channelob
 *
 * @ingroup Object
 *
 * @{
 */

AUG_INTERFACE(aug_channelob);

/**
 * All channels have non-blocking semantics.
 */

struct aug_channelobvtbl {
    AUG_VTBL(aug_channelob);
    aug_result (*close_)(aug_channelob*);
    aug_channelob* (*process_)(aug_channelob*, aug_bool*, aug_channelcb_t, aug_object*);
    aug_result (*seteventmask_)(aug_channelob*, unsigned short);
    unsigned (*getid_)(aug_channelob*);
    int (*eventmask_)(aug_channelob*);
};

#define aug_close(this_) \
    (this_)->vtbl_->close_(this_)

#define aug_process(this_, fork, cb, ob) \
    (this_)->vtbl_->process_(this_, fork, cb, ob)

#define aug_seteventmask(this_, mask) \
    (this_)->vtbl_->seteventmask_(this_, mask)

#define aug_getid(this_) \
    (this_)->vtbl_->getid_(this_)

#define aug_eventmask(this_) \
    (this_)->vtbl_->eventmask_(this_)

/** @} */

#if defined(__cplusplus)
namespace aug {
    template <>
    struct object_traits<aug_channelob> {
        typedef aug_channelobvtbl vtbl;
        static const char*
        id() AUG_NOTHROW
        {
            return aug_channelobid;
        }
    };
}

namespace aug {

    typedef aug::obref<aug_channelob> channelobref;

    inline aug_result
    close(aug::obref<aug_channelob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->close_(this_.get());
    }

    inline aug::smartob<aug_channelob>
    process(aug::obref<aug_channelob> this_, aug_bool& fork, aug_channelcb_t cb, aug::obref<aug_object> ob) AUG_NOTHROW
    {
        return aug::object_attach<aug_channelob>(this_.get()->vtbl_->process_(this_.get(), &fork, cb, ob.get()));
    }

    inline aug_result
    seteventmask(aug::obref<aug_channelob> this_, unsigned short mask) AUG_NOTHROW
    {
        return this_.get()->vtbl_->seteventmask_(this_.get(), mask);
    }

    inline unsigned
    getid(aug::obref<aug_channelob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->getid_(this_.get());
    }

    inline int
    eventmask(aug::obref<aug_channelob> this_) AUG_NOTHROW
    {
        return this_.get()->vtbl_->eventmask_(this_.get());
    }

    template <typename T>
    class channelob {

        aug_channelob channelob_;

        channelob(const channelob&);

        channelob&
        operator =(const channelob&);

        static void*
        cast_(aug_channelob* this_, const char* id) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->cast_(id));
        }
        static void
        retain_(aug_channelob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->retain_();
        }
        static void
        release_(aug_channelob* this_) AUG_NOTHROW
        {
            static_cast<T*>(this_->impl_)->release_();
        }
        static aug_result
        close_(aug_channelob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->close_();
        }
        static aug_channelob*
        process_(aug_channelob* this_, aug_bool* fork, aug_channelcb_t cb, aug_object* ob) AUG_NOTHROW
        {
            return aug::retget(static_cast<T*>(this_->impl_)->process_(fork, cb, ob));
        }
        static aug_result
        seteventmask_(aug_channelob* this_, unsigned short mask) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->seteventmask_(mask);
        }
        static unsigned
        getid_(aug_channelob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->getid_();
        }
        static int
        eventmask_(aug_channelob* this_) AUG_NOTHROW
        {
            return static_cast<T*>(this_->impl_)->eventmask_();
        }
        static const aug_channelobvtbl*
        vtbl()
        {
            static const aug_channelobvtbl local = {
                cast_,
                retain_,
                release_,
                close_,
                process_,
                seteventmask_,
                getid_,
                eventmask_
            };
            return &local;
        }
    public:
        explicit
        channelob(T* impl = 0)
        {
            this->channelob_.vtbl_ = this->vtbl();
            this->channelob_.impl_ = impl;
        }
        void
        reset(T* impl)
        {
            this->channelob_.impl_ = impl;
        }
        aug_channelob*
        get() AUG_NOTHROW
        {
            return &this->channelob_;
        }
        operator aug::obref<aug_channelob>() AUG_NOTHROW
        {
            return this->get();
        }
    };

    template <typename T>
    class basic_channelob {
        channelob<basic_channelob<T> > channelob_;
        T impl_;
        int refs_;
        explicit
        basic_channelob(const T& impl)
            : impl_(impl),
              refs_(1)
        {
            this->channelob_.reset(this);
        }
    public:
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_channelob>(id))
                return aug::object_retain<aug_object>(this->channelob_);
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
        aug_result
        close_() AUG_NOTHROW
        {
            return this->impl_.close_();
        }
        aug::smartob<aug_channelob>
        process_(aug_bool& fork, aug_channelcb_t cb, aug::obref<aug_object> ob) AUG_NOTHROW
        {
            return this->impl_.process_(fork, cb, ob);
        }
        aug_result
        seteventmask_(unsigned short mask) AUG_NOTHROW
        {
            return this->impl_.seteventmask_(mask);
        }
        unsigned
        getid_() AUG_NOTHROW
        {
            return this->impl_.getid_();
        }
        int
        eventmask_() AUG_NOTHROW
        {
            return this->impl_.eventmask_();
        }
        static aug::smartob<aug_channelob>
        create(const T& impl = T())
        {
            basic_channelob* ptr(new basic_channelob(impl));
            return aug::object_attach<aug_channelob>(ptr->channelob_);
        }
    };

    template <typename T>
    class scoped_channelob {
        channelob<scoped_channelob<T> > channelob_;
        T impl_;
    public:
        explicit
        scoped_channelob(const T& impl = T())
            : impl_(impl)
        {
            this->channelob_.reset(this);
        }
        aug::smartob<aug_object>
        cast_(const char* id) AUG_NOTHROW
        {
            if (aug::equalid<aug_object>(id) || aug::equalid<aug_channelob>(id))
                return aug::object_retain<aug_object>(this->channelob_);
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
        aug_result
        close_() AUG_NOTHROW
        {
            return this->impl_.close_();
        }
        aug::smartob<aug_channelob>
        process_(aug_bool& fork, aug_channelcb_t cb, aug::obref<aug_object> ob) AUG_NOTHROW
        {
            return this->impl_.process_(fork, cb, ob);
        }
        aug_result
        seteventmask_(unsigned short mask) AUG_NOTHROW
        {
            return this->impl_.seteventmask_(mask);
        }
        unsigned
        getid_() AUG_NOTHROW
        {
            return this->impl_.getid_();
        }
        int
        eventmask_() AUG_NOTHROW
        {
            return this->impl_.eventmask_();
        }
        aug_object*
        base() AUG_NOTHROW
        {
            return aug::obref<aug_object>(this->channelob_).get();
        }
        aug_channelob*
        get() AUG_NOTHROW
        {
            return this->channelob_.get();
        }
        operator aug::obref<aug_channelob>() AUG_NOTHROW
        {
            return this->channelob_;
        }
    };

    typedef aug::smartob<aug_channelob> channelobptr;
}
#endif /* __cplusplus */

#endif /* AUGOB_CHANNELOB_H */
