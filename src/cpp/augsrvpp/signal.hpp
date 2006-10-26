/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_SIGNAL_HPP
#define AUGSRVPP_SIGNAL_HPP

#include "augsrvpp/config.hpp"

#include "augsyspp/exception.hpp"

#include "augsrv/signal.h"

namespace aug {

    inline void
    signalhandler(void (*handler)(int))
    {
        verify(aug_signalhandler(handler));
    }

    inline void
    blocksignals()
    {
        verify(aug_blocksignals());
    }

    inline void
    unblocksignals()
    {
        verify(aug_unblocksignals());
    }

    class scoped_block {

        scoped_block(const scoped_block& rhs);

        scoped_block&
        operator =(const scoped_block& rhs);

    public:
        ~scoped_block() AUG_NOTHROW
        {
            if (-1 == aug_unblocksignals())
                perrinfo("aug_unblocksignals() failed");
        }

        scoped_block()
        {
            blocksignals();
        }
    };

    class scoped_unblock {

        scoped_unblock(const scoped_unblock& rhs);

        scoped_unblock&
        operator =(const scoped_unblock& rhs);

    public:
        ~scoped_unblock() AUG_NOTHROW
        {
            if (-1 == aug_blocksignals())
                perrinfo("aug_blocksignals() failed");
        }

        scoped_unblock()
        {
            unblocksignals();
        }
    };
}

#endif // AUGSRVPP_SIGNAL_HPP
