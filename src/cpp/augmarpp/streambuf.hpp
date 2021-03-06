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
#ifndef AUGMARPP_STREAMBUF_HPP
#define AUGMARPP_STREAMBUF_HPP

#include "augmarpp/compat.hpp"

#include "augctxpp/mpool.hpp"

#include "augmar/types.h"

#include "augctx/defs.h" // AUG_MAX

#include "augext/mar.h"

#include <iostream>

namespace aug {

    namespace detail {

        template <typename charT>
        inline std::streamsize
        streamdiff(const charT* begin, const charT* end)
        {
            return static_cast<std::streamsize>(end - begin);
        }

        class state {
        public:
            enum which { neutral, get, put };

        private:
            which which_;

        public:
            state()
                : which_(neutral)
            {
            }
            which
            operator ()(which w)
            {
                std::swap(which_, w);
                return w;
            }
            which
            operator ()() const
            {
                return which_;
            }
        };

        template <typename charT,
                  typename char_traitsT = std::char_traits<charT> >
        class buffer : public mpool_ops {
        public:
            typedef charT char_type;

            typedef char_traitsT char_traits_type;
            typedef typename char_traits_type::int_type int_type;
            typedef typename char_traits_type::pos_type pos_type;
            typedef typename char_traits_type::off_type off_type;

            enum { pbacksize = 4 };

        private:
            std::ios_base::openmode openmode_;
            std::streamsize outsize_;
            std::streamsize insize_;
            char_type* buffer_;

            buffer(const buffer& rhs);

            buffer&
            operator =(const buffer& rhs);

        public:
            ~buffer() AUG_NOTHROW
            {
                if (buffer_)
                    delete[] buffer_;
            }
            explicit
            buffer(std::ios_base::openmode mode, std::streamsize size)
                : openmode_(mode),
                  outsize_(0),
                  insize_(0),
                  buffer_(0)
            {
                if (mode & std::ios_base::out)
                    outsize_ = size;

                std::streamsize intotal(0);
                if (mode & std::ios_base::in)
                    intotal = (insize_ = size) + pbacksize;

                if (0 != (outsize_ + insize_))
                    buffer_ = new char_type[outsize_ + intotal];
            }
            std::ios_base::openmode
            openmode(std::ios_base::openmode mask = std::ios_base::in
                     | std::ios_base::out) const
            {
                return openmode_ & mask;
            }
            char_type*
            pbegin() const
            {
                return buffer_;
            }
            char_type*
            pend() const
            {
                return pbegin() + outsize_;
            }
            std::streamsize
            psize() const
            {
                return outsize_;
            }
            char_type*
            gbegin() const
            {
                return pend();
            }
            char_type*
            gbase() const
            {
                return gbegin() + pbacksize;
            }
            char_type*
            gend() const
            {
                return gbase() + insize_;
            }
            std::streamsize
            gsize() const
            {
                return insize_;
            }
        };
    }

    template <typename charT,
              typename char_traitsT = std::char_traits<charT> >
    class basic_marstreambuf
        : public std::basic_streambuf<charT, char_traitsT>,
          public mpool_ops {
        typedef std::basic_streambuf<charT, char_traitsT> base_type;
    public:
        typedef charT char_type;

        typedef unsigned size_type;
        typedef int ssize_type;

        typedef char_traitsT char_traits_type;
        typedef typename char_traits_type::int_type int_type;
        typedef typename char_traits_type::pos_type pos_type;
        typedef typename char_traits_type::off_type off_type;

    private:
        typedef detail::buffer<char_type> buffer_type;

        marptr mar_;
        buffer_type buffer_;
        detail::state state_;

        struct {
            char_type* eback;
            char_type* gptr;
            char_type* egptr;
        } gsaved_;

        void
        init()
        {
            gsaved_.eback = gsaved_.gptr = gsaved_.egptr = buffer_.gbase();
            base_type::setg(0, 0, 0);
            base_type::setp(0, 0);
        }
        ssize_type
        ioread(char_type* buf, size_type len)
        {
            aug_rsize rsize(aug_readmar(mar_.get(), buf,
                                        len * sizeof(char_type)));
            if (rsize < 0)
                return -1;

            return rsize / sizeof(char_type);
        }
        ssize_type
        iowrite(const char_type* buf, size_type len)
        {
            if (!len)
                return 0;

            aug_rsize rsize(aug_writemar(mar_.get(), buf,
                                         len * sizeof(char_type)));
            if (rsize < 0)
                return -1;

            return rsize / sizeof(char_type);
        }
        void
        gsave(detail::state::which old)
        {
            if (detail::state::get == old) {
                gsaved_.eback = base_type::eback();
                gsaved_.gptr = base_type::gptr();
                gsaved_.egptr = base_type::egptr();
            }
        }
        char_type*
        savepback()
        {
            std::streamsize len(pbacklen());
            char_type* to = buffer_.gbase() - len;
            char_type* from = base_type::gptr() - len;
            char_traits_type::copy(to, from, len);
            return to;
        }
        std::streamsize
        pbacklen() const
        {
            std::streamsize s(detail::streamdiff(base_type::eback(),
                                                 base_type::gptr()));
            return AUG_MIN(s, buffer_type::pbacksize);
        }
        int
        syncout()
        {
            if (detail::state::put != state_())
                return 0;

            char_type* begin = base_type::pbase();
            std::streamsize len(detail::streamdiff(begin, base_type::pptr()));
            std::streamsize num(iowrite(begin, len));

            if (-1 == num)
                return -1;

            base_type::pbump(-len);

            // GNU sets badbit if non-zero.

            return num == len ? 0 : -1;
        }
        int
        syncin()
        {
            if (detail::state::get != state_())
                return 0;

            return 0;
        }
        bool
        setout()
        {
            if (detail::state::put == state_())
                return true;
            else if (!buffer_.openmode(std::ios_base::out))
                return false;

            if (buffer_.openmode(std::ios_base::in))
                if (-1 == syncin())
                    return false;

            gsave(state_(detail::state::put));
            base_type::setg(0, 0, 0);
            base_type::setp(buffer_.pbegin(), buffer_.pend());
            return true;
        }
        bool
        setin()
        {
            if (detail::state::get == state_())
                return true;
            else if (!buffer_.openmode(std::ios_base::in))
                return false;

            if (buffer_.openmode(std::ios_base::out))
                if (-1 == syncout())
                    return false;

            state_(detail::state::get);
            base_type::setp(0, 0);
            base_type::setg(gsaved_.eback, gsaved_.gptr, gsaved_.egptr);
            return true;
        }
    protected:
        int_type
        underflow()
        {
            if (!setin())
                return char_traits_type::eof();

            if (base_type::gptr() < base_type::egptr())
                return char_traits_type::to_int_type(*base_type::gptr());

            char_type* pback = savepback();
            char_type* gbase = buffer_.gbase();
            ssize_type len(ioread(gbase, buffer_.gsize()));

            if (-1 == len || 0 == len)
                return char_traits_type::eof();

            setg(pback, gbase, gbase + len);
            return char_traits_type::to_int_type(*base_type::gptr());
        }
        int_type
        overflow(int_type c = char_traits_type::eof())
        {
            if (!setout() || -1 == syncout())
                return char_traits_type::eof();

            if (char_traits_type::eq_int_type(c, char_traits_type::eof()))
                return char_traits_type::not_eof(c);

            return sputc(c);
        }
        pos_type
        seekoff(off_type off, std::ios_base::seekdir way,
                std::ios_base::openmode which)
        {
            int whence;
            switch (way) {
            case std::ios_base::beg:
                whence = AUG_SET;
                break;
            case std::ios_base::cur:
                whence = AUG_CUR;
                break;
            case std::ios_base::end:
                whence = AUG_END;
                break;
            default:
                whence = 0;
            }
            aug_rsize rsize(aug_seekmar(mar_.get(), off, whence));
            if (rsize < 0)
                return -1;
            return rsize;
        }
        pos_type
        seekpos(pos_type pos, std::ios_base::openmode which)
        {
            aug_rsize rsize(aug_seekmar(mar_.get(), pos, AUG_SET));
            if (rsize < 0)
                return -1;
            return rsize;
        }
        int
        sync()
        {
            if (buffer_.openmode(std::ios_base::out))
                if (-1 == syncout())
                    return -1;

            if (buffer_.openmode(std::ios_base::in))
                if (-1 == syncin())
                    return -1;

            return 0;
        }
        std::streamsize
        xsgetn(char_type* s, std::streamsize n)
        {
            if (!n || !setin())
                return 0;

            std::streamsize tot(0);
            std::streamsize len(detail::streamdiff(base_type::gptr(),
                                                   base_type::egptr()));

            // If any characters exist in get area then copy as required to
            // the output buffer.

            if (0 < len) {

                if (len > n)
                    len = n;

                char_traits_type::copy(s, base_type::gptr(), len);
                s += len;
                tot += len;
                base_type::gbump(len);

                // No more characters.

                if (!(n -= len))
                    return tot;
            }

            // If number of remaining characters to read is equal to or
            // greater than get area then read direct to output buffer.

            if (n >= buffer_.gsize()) {

                if (-1 != (len = ioread(s, n)))
                    tot += len;

                return tot;
            }

            // Force read to internal get area.

            int_type ch(base_type::uflow());
            if (char_traits_type::eq_int_type(char_traits_type::eof(), ch))
                return tot;

            *s++ = char_traits_type::to_char_type(ch);
            ++tot;
            if (!--n)
                return tot;

            // If any characters exist in get area then copy to output buffer.

			len = detail::streamdiff(base_type::gptr(), base_type::egptr());
            if (0 < len) {

                if (len > n)
                    len = n;

                char_traits_type::copy(s, base_type::gptr(), len);
                s += len;
                tot += len;
                base_type::gbump(len);
            }
            return tot;
        }
        std::streamsize
        xsputn(const char_type* s, std::streamsize n)
        {
            if (!n || !setout())
                return 0;

            std::streamsize tot(0);
            std::streamsize len(detail::streamdiff(base_type::pptr(),
                                                   base_type::epptr()));

            // If any space exists in the put area then copy from input buffer
            // to put area as required.

            if (0 < len) {

                if (len > n)
                    len = n;

                char_traits_type::copy(base_type::pptr(), s, len);
                s += len;
                tot += len;
                base_type::pbump(len);

                // No more characters.

                if (!(n -= len))
                    return tot;
            }

            // The put area must be full; flush to output device.

            if (-1 == syncout())
                return tot;

            // If the number of remaining characters exceed the size of the
            // put area then write direct to the output device.

            if (n >= buffer_.psize()) {

                if (-1 != (len = iowrite(s, n)))
                    tot += len;

                return tot;
            }

            // Copy remaining characters to put area.

            char_traits_type::copy(base_type::pptr(), s, n);
            tot += n;
            base_type::pbump(n);
            return tot;
        }

    public:
        ~basic_marstreambuf() AUG_NOTHROW
        {
            if (is_open())
                close();
        }
        basic_marstreambuf(marref ref, std::ios_base::openmode mode,
                           std::streamsize size)
            : mar_(object_retain(ref)),
              buffer_(mode, size)
        {
            init();
        }
        int
        close()
        {
            if (!is_open())
                return 0;

            int ret(sync());

            if (buffer_.openmode(std::ios_base::out)
                && aug_syncmar(mar_.get()) < 0)
                ret = -1;

            mar_ = null;
            return ret;
        }
        bool
        is_open() const
        {
            return null != mar_;
        }
    };

    typedef basic_marstreambuf<char> marstreambuf;
}

#endif // AUGMARPP_STREAMBUF_HPP
