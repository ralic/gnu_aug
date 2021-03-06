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
#define AUGNET_BUILD
#include "augnet/fix.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augutil/xstr.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include <ctype.h>  /* isdigit() */

/* All fields (including those of data type data i.e. SecureData, RawData,
   SignatureData, XmlData, etc.) in a FIX message are terminated by a
   delimiter character.  The non-printing, ASCII "SOH" (#001, hex: 0x01,
   referred to in this document as <SOH>), is used for field termination.
   Messages are delimited by the "SOH" character following the CheckSum field.
   All messages begin with the "8=FIX.x.y<SOH>" string and terminate with
   "10=nnn<SOH>". */

/* General message format is composed of the standard header followed by the
   body followed by the standard trailer:

   The first three fields in the standard header are BeginString (tag #8)
   followed by BodyLength (tag #9) followed by MsgType (tag #35).

   The last field in the standard trailer is the CheckSum (tag #10). */

#define SOH_ "\01"
#define FIELD_SEP_ "="

#define BEGINSTRING_TAG_ "8"
#define BODYLENGTH_TAG_ "9"
#define CHECKSUM_TAG_ "10"

#define BEGINSTRING_PREFIX_ BEGINSTRING_TAG_ FIELD_SEP_
#define BODYLENGTH_PREFIX_ BODYLENGTH_TAG_ FIELD_SEP_
#define CHECKSUM_PREFIX_ CHECKSUM_TAG_ FIELD_SEP_

#define BEGINSTRING_PREFIX_SIZE_ (sizeof(BEGINSTRING_PREFIX_) - 1)
#define BODYLENGTH_PREFIX_SIZE_ (sizeof(BODYLENGTH_PREFIX_) - 1)
#define CHECKSUM_PREFIX_SIZE_ (sizeof(BODYLENGTH_PREFIX_) - 1)

#define BEGINSTRING_FIELD_ BEGINSTRING_TAG_ FIELD_SEP_ "FIX.x.y"
#define CHECKSUM_FIELD_ CHECKSUM_TAG_ FIELD_SEP_ "nnn"

#define HEAD_ BEGINSTRING_FIELD_ SOH_ BODYLENGTH_PREFIX_
#define TAIL_ CHECKSUM_FIELD_ SOH_

#define HEAD_SIZE_ (sizeof(HEAD_) - 1)
#define TAIL_SIZE_ (sizeof(TAIL_) - 1)

#define ENDOF_ (-2)

/* Maximum number of digits in a 32 bit integer. */

#define MAX_DIGITS_ 10

struct aug_fixstream_ {
    aug_mpool* mpool_;
    aug_fixcb_t cb_;
    aug_object* ob_;
    aug_xstr_t xstr_;
    size_t mlen_;
};

static aug_rsize
fixtoui_(const char* buf, unsigned* dst, size_t size, char delim)
{
    const char* it, * end = buf + size;
    unsigned digits, fact, value;

    for (it = buf; it != end; ++it)
        if (*it == delim)
            goto found;

    return 0;
 found:

    /* Verify the number of digits found does not exceed the maximum number of
       valid digits. */

    digits = (unsigned)(it - buf);
    if (MAX_DIGITS_ < digits) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("too many integer digits [%u]"), digits);
        return -1;
    }

    /* Given the string "1234", the integer value would be calculated as
       follows:

       4 * 1 + 3 * 10 + 2 * 100 + 1 * 1000 = 1234
    */

    for (fact = 1, value = 0; it-- != buf; fact *= 10) {

        if (!isdigit(*it)) {
            aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                            AUG_MSG("non-digit [%c] in integer"), *it);
            return -1;
        }

        value += (*it - '0') * fact;
    }

    *dst = value;
    return digits;
}

static aug_rsize
getsize_(const char* buf, size_t size)
{
    /* FIXME: Alter the fix parsing logic so that it does not assume a
       static header with a constant begin string value. */

    unsigned value = 0;
    aug_rsize rsize;

    /* In order for the buffer to contain the body length, it must be able
       to hold, at least, the standard leader, a single digit and a
       delimiter character. */

    if (size < HEAD_SIZE_ + sizeof('0') + sizeof(*SOH_))
        return 0;

    /* The beginning of the body length value is located immediately after the
       standard leader. */

    if ((rsize = fixtoui_(buf + HEAD_SIZE_, &value, size, *SOH_)) < 0)
        return -1;

    /* Return the total number of bytes in the message. */

    return HEAD_SIZE_ + rsize + sizeof(*SOH_) + value + TAIL_SIZE_;
}

static aug_rsize
getsum_(const char* buf, size_t size)
{
    char cu, ct, ch;
    aug_len sum;

    cu = buf[size - 2]; /* Units. */
    ct = buf[size - 3]; /* Tens. */
    ch = buf[size - 4]; /* Hundreds. */

    if (!isdigit(cu) || !isdigit(ct) || !isdigit(ch)) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("non-digit in checksum [%.3s]"),
                        buf + (size - 4));
        return -1;
    }

    sum = cu - '0';
    sum += (ct - '0') * 10;
    sum += (ch - '0') * 100;

    return sum;
}

AUGNET_API aug_fixstream_t
aug_createfixstream(aug_mpool* mpool, size_t size, aug_fixcb_t cb,
                    aug_object* ob)
{
    aug_fixstream_t stream;
    aug_xstr_t xstr;

    if (!(stream = aug_allocmem(mpool, sizeof(struct aug_fixstream_))))
        return NULL;

    if (!(xstr = aug_createxstr(mpool, 0 == size ? AUG_BUFSIZE : size))) {
        aug_freemem(mpool, stream);
        return NULL;
    }

    stream->mpool_ = mpool;
    stream->cb_ = cb;
    if ((stream->ob_ = ob))
        aug_retain(ob);
    stream->xstr_ = xstr;
    stream->mlen_ = 0;

    aug_retain(mpool);
    return stream;
}

AUGNET_API void
aug_destroyfixstream(aug_fixstream_t stream)
{
    aug_mpool* mpool = stream->mpool_;
    aug_destroyxstr(stream->xstr_);
    if (stream->ob_)
        aug_release(stream->ob_);
    aug_freemem(mpool, stream);
    aug_release(mpool);
}

AUGNET_API aug_rsize
aug_readfix_BI(aug_fixstream_t stream, aug_stream* src, size_t size)
{
    aug_rsize rsize, mlen;
    size_t blen;
    const char* ptr;

    /* Return on error or end of file. */

    /* EXCEPT: aug_readfix_BI -> aug_xstrread_BI; */

    rsize = aug_xstrread_BI(stream->xstr_, src, size);
    if (rsize < 0)
        return -1;

    if (0 == rsize) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EIO,
                        AUG_MSG("end of stream"));
        return -1;
    }

    /* Total number of buffered bytes. */

    blen = aug_xstrlen(stream->xstr_);
    ptr = aug_xstr(stream->xstr_);

    if (0 != stream->mlen_)
        goto body;

    /* Header and body parts of the message are alternately parsed until the
       entire buffer has been consumed. */

    for (;;) {

        if ((mlen = getsize_(ptr, size)) < 0)
            return -1;

        /* Not enough of the message has been read to determine the total
           message length. */

        if (0 == mlen)
            break;

        /* Cache total message length. */

        stream->mlen_ = mlen;

    body:
        if (blen < stream->mlen_)
            break;

        stream->cb_(ptr, stream->mlen_, stream->ob_);
        blen -= stream->mlen_;
        stream->mlen_ = 0;

        if (0 == blen) {
            aug_clearxstr(stream->xstr_);
            break;
        }

        aug_xstrcpysn(stream->xstr_, ptr + stream->mlen_, blen);
        ptr = aug_xstr(stream->xstr_);
    }

    return rsize;
}

AUGNET_API aug_result
aug_finishfix(aug_fixstream_t stream)
{
    /* Test for any partially buffered messages. */

    size_t size = aug_xstrlen(stream->xstr_);
    aug_result result;

    if (size) {

        aug_clearxstr(stream->xstr_);
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EIO,
                        AUG_MSG("fix stream not empty, [%d] bytes"),
                        (int)size);
        result = -1;

    } else
        result = 0;

    stream->mlen_ = 0;
    return result;
}

AUGNET_API aug_result
aug_checkfix(struct aug_fixstd_* fixstd, const char* buf, size_t size)
{
    const char* ptr = buf;
    aug_rsize sum1;
    aug_len sum2;

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ^^
    */

    if (0 != memcmp(ptr, BEGINSTRING_PREFIX_, BEGINSTRING_PREFIX_SIZE_)) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("invalid beginstring tag"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ..^^^^^^^
    */

    ptr += BEGINSTRING_PREFIX_SIZE_;
    memcpy(fixstd->fixver_, ptr, AUG_FIXVERLEN);
    fixstd->fixver_[AUG_FIXVERLEN] = '\0';

    /* 8=FIX.4.2^9=5^35=D^10=181^
       .........^
    */

    ptr += AUG_FIXVERLEN;
    if (*SOH_ != *ptr++) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("beginstring field delimiter not found"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ..........^^
    */

    if (0 != memcmp(ptr, BODYLENGTH_PREFIX_, BODYLENGTH_PREFIX_SIZE_)) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("invalid bodylength tag"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       .............^
    */

    /* Move one character past the SOH delimiter. */

    /* Note: the SOH delimiter is known to exist - getsum_() was used when
       reading the message. */

    ptr += BODYLENGTH_PREFIX_SIZE_;
    while (*SOH_ != *ptr++)
        ;

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ..............^^^^^
    */

    /* Set body pointer and size. */

    fixstd->body_ = ptr;
    ptr = buf + size - TAIL_SIZE_;
    fixstd->size_ = ptr - fixstd->body_;

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ...................^^^
    */

    if (0 != memcmp(ptr, CHECKSUM_PREFIX_, CHECKSUM_PREFIX_SIZE_)) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("invalid checksum tag"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ...................      ^
    */

    if (*SOH_ != buf[size - 1]) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("checksum field delimiter not found"));
        return -1;
    }

    /* 8=FIX.4.2^9=5^35=D^10=181^
       ......................^^^.
    */

    /* Get sum contained within message. */

    if ((sum1 = getsum_(buf, size)) < 0)
        return -1;

    /* Calculate message checksum. */

    sum2 = aug_checksum(buf, size - TAIL_SIZE_);

    if (sum1 != sum2) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("invalid checksum [%03d], expected [%03d]"),
                        sum1, sum2);
        return -1;
    }

    return 0;
}

AUGNET_API aug_len
aug_checksum(const char* buf, size_t size)
{
    const char* end = buf + size;
    unsigned long sum = 0;
    for (; buf != end; ++buf)
        sum += *buf;
    return sum % 256;
}

AUGNET_API aug_rsize
aug_fixfield(struct aug_fixfield_* field, const char* buf, size_t size)
{
    unsigned tag;
    aug_result digits;
    const char* it, * end = buf + size;

    /* Extract tag value from buffer. */

    if ((digits = fixtoui_(buf, &tag, size, '=')) < 0)
        return -1;

    if (0 == digits) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("tag not found"));
        return -1;
    }

    field->tag_ = tag;

    it = buf + digits + sizeof(*SOH_);
    field->value_ = it;

    /* Locate field delimiter. */

    for (; it != end; ++it)
        if (*it == *SOH_)
            goto found;

    aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                    AUG_MSG("field [%d] delimiter not found"),
                    (int)field->tag_);
    return -1;
 found:

    field->size_ = (aug_len)(it - field->value_);

    /* Return number of bytes consumed. */

    return (ssize_t)((it - buf) + sizeof(*SOH_));
}
