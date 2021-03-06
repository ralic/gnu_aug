/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGSYSPP_ENDPOINT_HPP
#define AUGSYSPP_ENDPOINT_HPP

#include "augsyspp/socket.hpp"

#include "augctxpp/mpool.hpp"

namespace aug {

    inline aug_endpoint&
    clear(aug_endpoint& ep)
    {
        memset(&ep, 0, sizeof(aug_endpoint));
        return ep;
    }

    inline aug_endpoint&
    setfamily(aug_endpoint& ep, short family)
    {
        switch (ep.un_.family_ = family) {
        case AF_INET:
            ep.len_ = sizeof(sockaddr_in);
            break;
        case AF_INET6:
            ep.len_ = sizeof(sockaddr_in6);
            break;
        default:
            ep.len_ = AUG_MAXADDRLEN;
        }
        return ep;
    }

    inline aug_endpoint&
    setport(aug_endpoint& ep, unsigned short port)
    {
        ep.un_.all_.port_ = port;
        return ep;
    }

    inline aug_endpoint&
    setsockaddr(aug_endpoint& ep, const sockaddr_in& ipv4)
    {
        ep.len_ = sizeof(sockaddr_in);
        memcpy(&ep.un_.ipv4_, &ipv4, sizeof(ep.un_.ipv4_));
        return ep;
    }

#if HAVE_IPV6
    inline aug_endpoint&
    setsockaddr(aug_endpoint& ep, const sockaddr_in6& ipv6)
    {
        ep.len_ = sizeof(sockaddr_in6);
        memcpy(&ep.un_.ipv6_, &ipv6, sizeof(ep.un_.ipv6_));
        return ep;
    }
#endif // HAVE_IPV6

    inline short
    family(const aug_endpoint& ep)
    {
        return ep.un_.family_;
    }

    inline socklen_t
    len(const aug_endpoint& ep)
    {
        return ep.len_;
    }

    inline unsigned short
    port(const aug_endpoint& ep)
    {
        return ep.un_.all_.port_;
    }

    class endpoint : public mpool_ops {
    public:
        typedef aug_endpoint ctype;
    private:
        aug_endpoint ep_;

    public:
        endpoint(const null_&) AUG_NOTHROW
        {
            clear(ep_);
        }

        explicit
        endpoint(const addrinfo& addr)
        {
            getendpoint(addr, ep_);
        }

        explicit
        endpoint(const aug_inetaddr& addr, unsigned short port = 0)
        {
            setport(ep_, port);
            setinetaddr(ep_, addr);
        }

        endpoint&
        operator =(const null_&) AUG_NOTHROW
        {
            clear(ep_);
            return *this;
        }

        operator aug_endpoint&()
        {
            return ep_;
        }

        operator const aug_endpoint&() const
        {
            return ep_;
        }
    };
}

inline bool
isnull(const aug_endpoint& ep)
{
    return 0 == aug::family(ep);
}

inline bool
isnull(const aug::endpoint& ep)
{
    return 0 == aug::family(ep);
}

#endif // AUGSYSPP_ENDPOINT_HPP
