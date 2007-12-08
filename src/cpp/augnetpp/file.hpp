/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_FILE_HPP
#define AUGNETPP_FILE_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/object.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augnet/file.h"

#include "augutil/list.h"

#include "augsys/errno.h"
#include "augsys/log.h"

#include <memory> // auto_ptr<>

namespace aug {

    template <bool (*T)(aug_object*, int)>
    int
    filecb(aug_object* user, int fd) AUG_NOTHROW
    {
        try {
            return T(user, fd) ? 1 : 0;
        } AUG_SETERRINFOCATCH;

        /**
           Do not remove the file unless explicitly asked to.
        */

        return 1;
    }

    template <typename T, bool (T::*U)(int)>
    int
    filememcb(aug_object* user, int fd) AUG_NOTHROW
    {
        try {
            return (obtoaddr<T*>(user)->*U)(fd) ? 1 : 0;
        } AUG_SETERRINFOCATCH;
        return 1;
    }

    template <typename T>
    int
    filememcb(aug_object* user, int fd) AUG_NOTHROW
    {
        try {
            return obtoaddr<T*>(user)->filecb(fd) ? 1 : 0;
        } AUG_SETERRINFOCATCH;
        return 1;
    }

    class files {
    public:
        typedef aug_files ctype;
    private:

        friend class file;

        aug_files files_;

        files(const files&);

        files&
        operator =(const files&);

    public:
        ~files() AUG_NOTHROW
        {
            if (-1 == aug_destroyfiles(&files_))
                perrinfo("aug_destroyfiles() failed");
        }

        files()
        {
            AUG_INIT(&files_);
        }

        operator aug_files&()
        {
            return files_;
        }

        operator const aug_files&() const
        {
            return files_;
        }
    };

    inline void
    insertfile(aug_files& files, fdref ref, aug_filecb_t cb,
               obref<aug_object> user)
    {
        verify(aug_insertfile(&files, ref.get(), cb, user.get()));
    }

    inline void
    insertfile(aug_files& files, fdref ref, aug_filecb_t cb, const null_&)
    {
        verify(aug_insertfile(&files, ref.get(), cb, 0));
    }

    template <typename T>
    void
    insertfile(aug_files& files, fdref ref, T& x)
    {
        smartob<aug_addrob> obj(createaddrob(&x, 0));
        verify(aug_insertfile(&files, ref.get(), filememcb<T>, obj.base()));
    }

    template <typename T>
    void
    insertfile(aug_files& files, fdref ref, std::auto_ptr<T>& x)
    {
        smartob<aug_addrob> obj(createaddrob(x));
        verify(aug_insertfile(&files, ref.get(), filememcb<T>, obj.base()));
    }

    inline void
    removefile(aug_files& files, fdref ref)
    {
        verify(aug_removefile(&files, ref.get()));
    }

    inline void
    foreachfile(aug_files& files)
    {
        verify(aug_foreachfile(&files));
    }

    inline bool
    emptyfiles(aug_files& files)
    {
        return AUG_EMPTY(&files);
    }
}

#endif // AUGNETPP_FILE_HPP
