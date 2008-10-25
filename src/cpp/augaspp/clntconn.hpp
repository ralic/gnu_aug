/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_CLNTCONN_HPP
#define AUGASPP_CLNTCONN_HPP

#include "augaspp/buffer.hpp"
#include "augaspp/conn.hpp"

namespace aug {

    class clntconn : public conn_base, public rwtimer_base {

        mod_handle sock_;
        aug::connimpl impl_;
        buffer buffer_;
        rwtimer rwtimer_;

        // conn_base.

        mod_handle&
        do_get();

        const mod_handle&
        do_get() const;

        const sessionptr&
        do_session() const;

        void
        do_send(chanref chan, const void* buf, size_t size,
                const timeval& now);

        void
        do_sendv(chanref chan, blobref blob, const timeval& now);

        bool
        do_accepted(const std::string& name, const timeval& now);

        void
        do_connected(const std::string& name, const timeval& now);

        bool
        do_process(chanref chan, unsigned short events, const timeval& now);

        void
        do_shutdown(chanref chan, unsigned flags, const timeval& now);

        void
        do_teardown(const timeval& now);

        bool
        do_authcert(const char* subject, const char* issuer);

        std::string
        do_peername(chanref chan) const;

        sockstate
        do_state() const;

        // rwtimer_base.

        void
        do_timercb(idref id, unsigned& ms);

        void
        do_setrwtimer(unsigned ms, unsigned flags);

        bool
        do_resetrwtimer(unsigned ms, unsigned flags);

        bool
        do_resetrwtimer(unsigned flags);

        bool
        do_cancelrwtimer(unsigned flags);

    public:
        ~clntconn() AUG_NOTHROW;

        clntconn(mpoolref mpool, const sessionptr& session, void* user,
                 timers& timers, unsigned id);
    };
}

#endif // AUGASPP_CLNTCONN_HPP
