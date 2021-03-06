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
#ifndef AUGCTX_ERRNO_H
#define AUGCTX_ERRNO_H

/**
 * @file augctx/errno.h
 *
 * Portable errno support.
 */

#include "augctx/config.h"

#include <errno.h>

#if defined(_WIN32)

# define AUG_ERRNOBASE 100

# define EWOULDBLOCK EAGAIN
# define EINPROGRESS (AUG_ERRNOBASE + 0)
# define EALREADY (AUG_ERRNOBASE + 1)
# define ENOTSOCK (AUG_ERRNOBASE + 2)
# define EDESTADDRREQ (AUG_ERRNOBASE + 3)
# define EMSGSIZE (AUG_ERRNOBASE + 4)
# define EPROTOTYPE (AUG_ERRNOBASE + 5)
# define ENOPROTOOPT (AUG_ERRNOBASE + 6)
# define EPROTONOSUPPORT (AUG_ERRNOBASE + 7)
# define ESOCKTNOSUPPORT (AUG_ERRNOBASE + 8)
# define EOPNOTSUPP (AUG_ERRNOBASE + 9)
# define EPFNOSUPPORT (AUG_ERRNOBASE + 10)
# define EAFNOSUPPORT (AUG_ERRNOBASE + 11)
# define EADDRINUSE (AUG_ERRNOBASE + 12)
# define EADDRNOTAVAIL (AUG_ERRNOBASE + 13)
# define ENETDOWN (AUG_ERRNOBASE + 14)
# define ENETUNREACH (AUG_ERRNOBASE + 15)
# define ENETRESET (AUG_ERRNOBASE + 16)
# define ECONNABORTED (AUG_ERRNOBASE + 17)
# define ECONNRESET (AUG_ERRNOBASE + 18)
# define ENOBUFS (AUG_ERRNOBASE + 19)
# define EISCONN (AUG_ERRNOBASE + 20)
# define ENOTCONN (AUG_ERRNOBASE + 21)
# define ESHUTDOWN (AUG_ERRNOBASE + 22)
# define ETOOMANYREFS (AUG_ERRNOBASE + 23)
# define ETIMEDOUT (AUG_ERRNOBASE + 24)
# define ECONNREFUSED (AUG_ERRNOBASE + 25)
# define ELOOP (AUG_ERRNOBASE + 26)
# define EHOSTDOWN (AUG_ERRNOBASE + 27)
# define EHOSTUNREACH (AUG_ERRNOBASE + 28)
# define EPROCLIM (AUG_ERRNOBASE + 29)
# define EUSERS (AUG_ERRNOBASE + 30)
# define EDQUOT (AUG_ERRNOBASE + 31)
# define ESTALE (AUG_ERRNOBASE + 32)
# define EREMOTE (AUG_ERRNOBASE + 33)

# define ESYSNOTREADY (AUG_ERRNOBASE + 100)
# define EVERNOTSUPPORTED (AUG_ERRNOBASE + 101)
# define ENOTINITIALISED (AUG_ERRNOBASE + 102)
# define EDISCON (AUG_ERRNOBASE + 103)
# define EHOSTNOTFOUND (AUG_ERRNOBASE + 104)
# define ETRYAGAIN (AUG_ERRNOBASE + 105)
# define ENORECOVERY (AUG_ERRNOBASE + 106)
# define ENODATA (AUG_ERRNOBASE + 107)
# define EIOINCOMPLETE (AUG_ERRNOBASE + 108)
# define EIOPENDING (AUG_ERRNOBASE + 109)
# define EOPERATIONABORTED (AUG_ERRNOBASE + 110)

/**
 * Where possible, map Win32 specific error code to equivalent posix value.
 *
 * @param win32 Win32 error code.
 *
 * @return Equivalent posix errno value.
 */

AUGCTX_API int
aug_win32posix(unsigned long win32);

#endif /* _WIN32 */

#endif /* AUGCTX_ERRNO_H */
