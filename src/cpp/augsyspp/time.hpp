/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_TIME_HPP
#define AUGSYSPP_TIME_HPP

#include "augsyspp/exception.hpp"

#include "augsys/time.h"

namespace aug {

    inline timeval&
    gettimeofday(timeval& tv, struct timezone& tz)
    {
        verify(aug_gettimeofday(&tv, &tz));
        return tv;
    }

    inline timeval&
    gettimeofday(timeval& tv)
    {
        verify(aug_gettimeofday(&tv, 0));
        return tv;
    }

    inline tm&
    gmtime(const time_t& clock, tm& res)
    {
        return *verify(aug_gmtime(&clock, &res));
    }

    inline tm&
    gmtime(tm& res)
    {
        return gmtime(time(0), res);
    }

    inline tm&
    localtime(const time_t& clock, tm& res)
    {
        return *verify(aug_localtime(&clock, &res));
    }

    inline tm&
    localtime(tm& res)
    {
        return localtime(time(0), res);
    }

    inline timeval&
    mstotv(timeval& tv, unsigned ms)
    {
        return *aug_mstotv(&tv, ms);
    }

    inline unsigned
    tvtoms(const timeval& tv)
    {
        return aug_tvtoms(&tv);
    }

    inline unsigned long
    tvtousec(const timeval& tv)
    {
        return aug_tvtousec(&tv);
    }

    inline timeval&
    tvadd(timeval& dst, const timeval& src)
    {
        return *aug_tvadd(&dst, &src);
    }

    inline timeval&
    tvsub(timeval& dst, const timeval& src)
    {
        return *aug_tvsub(&dst, &src);
    }
}

#endif // AUGSYSPP_TIME_HPP
