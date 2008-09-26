/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGTYPES_H
#define AUGTYPES_H

#include "augconfig.h"

/**
 * @defgroup Types Types
 */

/**
 * @defgroup TypesBool Boolean Values
 *
 * @ingroup Types
 *
 * @{
 */

/**
 * Boolean type.
 *
 * C++ enums are not guaranteed to have sizeof(int) so int is used instead.
 */

typedef int aug_bool;

#define AUG_TRUE  (1 == 1)
#define AUG_FALSE (1 != 1)

/** @} */

/**
 * Equivalent to socklen_t.
 */

typedef unsigned aug_len_t;

/**
 * @defgroup TypesResult Result Codes
 *
 * @ingroup Types
 *
 * Standard integer return codes.
 *
 * @{
 */

/**
 * Standard result type.
 *
 * The @ref aug_result type should be used instead of int, where functions
 * return 0 or -1 to indicate success or failure.  The type helps to
 * communicate intent.
 *
 * Negative aug_result values indicate exceptions.  Not all exceptions are
 * errors.  The posix errno values @ref EINTR and @ref EWOULDBLOCK are good
 * examples of this.  Equivalents of these are @ref AUG_FAILINTR and @ref
 * AUG_FAILBLOCK.
 */

#if ENABLE_STRICT

struct aug_strict_ {
    long val_;
};

typedef struct aug_strict_ aug_result;
typedef struct aug_strict_ aug_rint;
typedef struct aug_strict_ aug_rsize;

static inline aug_result
aug_mkresult_(long val)
{
    aug_result x;
    x.val_ = val;
    return x;
}

# define AUG_MKRESULT(x) aug_mkresult_(x)
# define AUG_RESULT(x)   (x).val_

#else /* !ENABLE_STRICT */

typedef int aug_result;
typedef int aug_rint;
typedef ssize_t aug_rsize;

# define AUG_MKRESULT(x) x
# define AUG_RESULT(x)   x

#endif /* !ENABLE_STRICT */

#define AUG_SUCCESS   AUG_MKRESULT( 0)
#define AUG_FAILERROR AUG_MKRESULT(-1)

/* Non-error exceptions.  aug_clearerrinfo() should be called before returning
   athese .*/

#define AUG_FAILNONE  AUG_MKRESULT(-2) /* ENOENT */
#define AUG_FAILINTR  AUG_MKRESULT(-3) /* EINTR, WSAEINTR */
#define AUG_FAILBLOCK AUG_MKRESULT(-4) /* EWOULDBLOCK, WSAEWOULDBLOCK */

#define AUG_ISFAIL(x)    (AUG_RESULT(x) < 0)
#define AUG_ISSUCCESS(x) (!AUG_ISFAIL(x))

#define AUG_ISERROR(x)  (-1 == AUG_RESULT(x))
#define AUG_ISNONE(x)   (-2 == AUG_RESULT(x))
#define AUG_ISINTR(x)   (-3 == AUG_RESULT(x))
#define AUG_ISBLOCK(x)  (-4 == AUG_RESULT(x))

#define aug_verify(x)                                                   \
    do {                                                                \
        aug_result aug_tmp;                                             \
        if (AUG_ISFAIL(aug_tmp = (x)))                                  \
            return aug_tmp;                                             \
    } while (0)

/** @} */

/**
 * @defgroup TypesException Exception Codes
 *
 * @ingroup Types
 *
 * When returned as status codes, the negated form of the following exception
 * codes should be used.
 *
 * @{
 */

/**
 * Assertion failure.
 */

#define AUG_EASSERT   2

/**
 * Authentication failure.
 */

#define AUG_EAUTH     3

/**
 * Configuration error.
 */

#define AUG_ECONFIG   4

/**
 * Domain-level error.
 */

#define AUG_EDOMAIN   5

/**
 * End-of sequence.
 */

#define AUG_EENDOF    6

/**
 * C++ exception.
 */

#define AUG_EEXCEPT   7

/**
 * Either exists or does not exist.
 */

#define AUG_EEXIST    8

/**
 * Formatting error.
 */

#define AUG_EFORMAT   9

/**
 * Invalid argument.
 */

#define AUG_EINVAL   10

/**
 * IO error.
 */

#define AUG_EIO      11

/**
 * Limit exceeded.
 */

#define AUG_ELIMIT   12

/**
 * Null pointer.
 */

#define AUG_ENULL    13

/**
 * Out of memory.
 */

#define AUG_EMEMORY  14

/**
 * Parse error.
 */

#define AUG_EPARSE   15

/**
 * Permission denied.
 */

#define AUG_EPERM    16

/**
 * Range or bounds exceeded.
 */

#define AUG_ERANGE   17

/**
 * Invalid state transition.
 */

#define AUG_ESTATE   18

/**
 * Operation not supported.
 */

#define AUG_ESUPPORT 19

/**
 * Type-related error.
 */

#define AUG_ETYPE    20

/**
 * Timeout.
 */

#define AUG_ETIMEOUT 21

/** @} */

#endif /* AUGTYPES_H */
