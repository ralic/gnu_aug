/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_FILE_HPP
#define AUGUTILPP_FILE_HPP

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"

#include "augutil/file.h"

#include "augsys/errno.h"
#include "augsys/log.h"

namespace aug {

    class confcb_base {

        virtual void
        do_callback(const char* name, const char* value) = 0;

    public:
        virtual
        ~confcb_base() NOTHROW
        {
        }

        void
        callback(const char* name, const char* value)
        {
            do_callback(name, value);
        }

        void
        operator ()(const char* name, const char* value)
        {
            do_callback(name, value);
        }
    };

    namespace detail {

        inline int
        confcb(const struct aug_var* arg, const char* name, const char* value)
        {
            try {
                confcb_base* ptr = static_cast<
                    confcb_base*>(aug_getvarp(arg));
                ptr->callback(name, value);
                return 0;
            } AUG_SETERRINFOCATCH;
            return -1;
        }
    }

    inline void
    readconf(const char* path, confcb_base& cb)
    {
        var v(&cb);
        if (-1 == aug_readconf(path, detail::confcb, cptr(v)))
            throwerrinfo("aug_readconf() failed");
    }
}

#endif // AUGUTILPP_FILE_HPP
