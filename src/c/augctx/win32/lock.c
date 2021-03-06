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
#include <assert.h>
#include <stdlib.h> /* malloc() */

#if !defined(WIN32_LEAN_AND_MEAN)
# define WIN32_LEAN_AND_MEAN
#endif /* !WIN32_LEAN_AND_MEAN */
#include <windows.h>

static LPCRITICAL_SECTION cs_ = NULL;

AUG_EXTERNC aug_bool
aug_initlock_(void)
{
    /* Create singleton mutex object. */

    LPCRITICAL_SECTION cs = malloc(sizeof(*cs));
    if (!cs)
        return AUG_FALSE;

	/* In low memory situations, InitializeCriticalSection() can raise a
       STATUS_NO_MEMORY exception. */

    InitializeCriticalSection(cs);

    /* Explicit MemoryBarrier() not required: MSDN states that a full memory
       barrier is generated by InterlockedCompareExchangePointer(). */

    if (InterlockedCompareExchangePointer((volatile PVOID*)&cs_, cs, NULL)) {

        /* Mutex already initialised. */

        DeleteCriticalSection(cs);
        free(cs);
    }
    return AUG_TRUE;
}

AUGCTX_API void
aug_lock(void)
{
	/* In low memory situations, EnterCriticalSection can raise a
       STATUS_INVALID_HANDLE exception. */

    assert(cs_);
    EnterCriticalSection(cs_);
}

AUGCTX_API void
aug_unlock(void)
{
    assert(cs_);
    LeaveCriticalSection(cs_);
}
