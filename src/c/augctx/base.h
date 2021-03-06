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
#ifndef AUGCTX_BASE_H
#define AUGCTX_BASE_H

#include "augctx/ctx.h"
#include "augtypes.h"

/**
 * @file augctx/base.h
 *
 * Context functions.
 */

/**
 * Initialise library for calling thread.
 *
 * Multiple calls to aug_init() can be safely made from a single thread.  This
 * allows initialisation from functions such as DllMain().  Each call must be
 * matched by a call to aug_term() when the thread is finished with the
 * library.  An internal reference count tracks multiple calls, and
 * termination only happens when this count reaches zero.
 *
 * A thread must ensure the thread-local context is set after calling
 * aug_init(), as many library functions will depend on it.
 *
 * @return Either @ref AUG_TRUE or @ref AUG_FALSE.  @ref TypesResult is not
 * used because the error handler is not initialised at this point.
 */

AUGCTX_API aug_bool
aug_init(void);

/**
 * Terminate use of library.
 *
 * Informs the library that the calling thread has finished with it, and that
 * any associated resources may now be freed.
 *
 * When freeing resources, aug_term() will release the thread-local context.
 *
 * If the thread-local context has not been initialised, then this function
 * will have no effect.  This is required to handle the situation highlighted
 * in the following paragraph.
 *
 * On Windows, signal or console handlers do not run on the main thread.  The
 * thread-local context is therefore unlikely to be available during execution
 * of these handlers.  If the handler exits the application, perhaps via an
 * abort(), then the atexit() handlers will be called from this thread, which
 * may include the atexit() handler installed by aug_autotlx().
 */

AUGCTX_API void
aug_term(void);

/**
 * Set thread-local context.
 *
 * The context will be retained in thread-local storage.
 *
 * @param ctx The context to be retained.
 */

AUGCTX_API void
aug_settlx(aug_ctx* ctx);

/**
 * Set thread-local with basic context.
 *
 * Use aug_createbasicctx() to create a context, and assign it to the
 * thread-local.
 *
 * @return See @ref TypesResult.
 */

AUGCTX_API aug_result
aug_setbasictlx(aug_mpool* mpool);

/**
 * Obtain reference to thread-local context.  Null is returned if no
 * thread-local context exists.
 *
 * The caller must release the reference when finished with it.
 *
 * @return A retained context reference.
 */

AUGCTX_API aug_ctx*
aug_gettlx(void);

/** @{ */

/**
 * Obtain reference to thread-local context, without retaining a reference to
 * it.  Null is returned if no thread-local context exists.
 *
 * The returned reference is not retained.  The caller must _not_, therefore,
 * release it.  This function can also be used to test the existence of a
 * thread-local context.
 *
 * @return A borrowed context reference or null.
 */

#define aug_tlx aug_tlx_()

AUGCTX_API aug_ctx*
aug_tlx_(void);

/** @} */

/**
 * Obtain thread-local error.  Null is returned if no thread-local context
 * exists.
 *
 * @return The thread-local error or null.
 */

#define aug_tlerr aug_tlerr_()

AUGCTX_API struct aug_errinfo*
aug_tlerr_(void);

/**
 * Initialise with basic context.
 *
 * Initialise and set, if not already set, basic context using
 * aug_setbasictlx().
 *
 * @return Either @ref AUG_TRUE or @ref AUG_FALSE.  @ref TypesResult is not
 * used because the error handler is not initialised at this point.
 */

AUGCTX_API aug_bool
aug_inittlx(void);

/**
 * Convenience wrapper for aug_init(), aug_createbasicctx() and aug_term().
 *
 * If one does not exist, a thread-local context is created using
 * aug_createbasicctx().  Library termination is scheduled using atexit().
 *
 * @return Either @ref AUG_TRUE or @ref AUG_FALSE.  @ref TypesResult is not
 * used because the error handler is not initialised at this point.
 */

AUGCTX_API aug_bool
aug_autotlx(void);

#endif /* AUGCTX_BASE_H */
