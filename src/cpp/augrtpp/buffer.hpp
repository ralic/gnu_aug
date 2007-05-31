/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_BUFFER_HPP
#define AUGRTPP_BUFFER_HPP

#include "augrtpp/config.hpp"
#include "augsyspp/types.hpp"
#include "augnetpp/writer.hpp"

#include <vector>

#define AUG_VARBUFFER 1

namespace aug {

    struct vecarg {
        std::vector<char> vec_;
        size_t begin_, end_;
        explicit
        vecarg(size_t size)
            : vec_(size),
              begin_(0),
              end_(0)
        {
        }
    };

    class buffer {
        writer writer_;
    public:
        vecarg arg_;
        bool usevec_;

        ~buffer() AUG_NOTHROW;

        explicit
        buffer(size_t size = 1024);

        void
        append(const aug_var& var);

        void
        append(const void* buf, size_t size);

        bool
        writesome(fdref ref);

        bool
        empty() const
        {
            return writer_.empty();
        }
    };
}

#endif // AUGRTPP_BUFFER_HPP