/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augrtpp.hpp"

#include "augutilpp.hpp"
#include "augsyspp.hpp"

#include <fstream>

using namespace augrt;
using namespace std;

namespace {

    const char MSG[] = "hello, world!\r\n";

    const void*
    buf(void* arg, size_t* size)
    {
        if (size)
            *size = sizeof(MSG) - 1;
        return MSG;
    }

    const struct augrt_vartype vartype = {
        NULL,
        buf
    };

    const struct augrt_var var = {
        &vartype,
        NULL
    };

    void
    dosend(object& sock)
    {
        send(sock, MSG, sizeof(MSG) - 1);
    }

    void
    dosendv(object& sock)
    {
        sendv(sock, var);
    }

    void
    pushxy(map<unsigned long, unsigned long>& xy,
           const vector<unsigned long>& usecs)
    {
        if (usecs.empty())
            return;

        vector<unsigned long>::const_iterator it(usecs.begin()),
            end(usecs.end());

        unsigned long first(*it);
        for (++it; it != end; ++it) {

            unsigned long second(*it - first);
            first = *it;
            xy.insert(make_pair(first, second));
        }
    }

    void
    writexy(const map<unsigned long, unsigned long>& xy)
    {
        ofstream os("bench.dat");
        map<unsigned long, unsigned long>::const_iterator it(xy.begin()),
            end(xy.end());
        for (; it != end; ++it)
            os << static_cast<double>(it->first) / 1000000.0
               << ' ' << static_cast<double>(it->second) / 1000.0
               << endl;
    }

    struct state {
        string tok_;
        unsigned tosend_, torecv_;
        vector<unsigned long> usecs_;
        explicit
        state(int echos)
            : tosend_(echos),
              torecv_(echos)
        {
        }
    };

    struct eachline {
        void (*fn_)(object&);
        object sock_;
        const aug::ptimer* ptimer_;
        vector<unsigned long>& usecs_;
        explicit
        eachline(void (*fn)(object&), const object& sock,
                 const aug::ptimer& ptimer, vector<unsigned long>& usecs)
            : fn_(fn),
              sock_(sock),
              ptimer_(&ptimer),
              usecs_(usecs)
        {
        }
        void
        operator ()(std::string& tok)
        {
            state& s(*sock_.user<state>());
            usecs_.push_back(ptimer_->now());
            if (0 == --s.torecv_)
                shutdown(sock_, 0);
            else if (0 < s.tosend_--)
                fn_(sock_);
        }
    };

    struct benchsession : basic_session {
        void (*send_)(object&);
        unsigned conns_, estab_, echos_;
        size_t bytes_;
        aug::ptimer ptimer_;
        map<unsigned long, unsigned long> xy_;
        bool
        do_start(const char* sname)
        {
            writelog(AUGRT_LOGINFO, "starting...");

            if (atoi(augrt::getenv("session.bench.sendv", "1"))) {
                send_ = dosendv;
                augrt_writelog(AUGRT_LOGINFO, "sendv: yes");
            } else {
                send_ = dosend;
                augrt_writelog(AUGRT_LOGINFO, "sendv: no");
            }

            const char* serv = augrt::getenv("session.bench.serv");
            if (!serv)
                return false;

            const char* host = augrt::getenv("session.bench.host",
                                             "localhost");
            conns_ = atoi(augrt::getenv("session.bench.conns", "100"));
            echos_ = atoi(augrt::getenv("session.bench.echos", "1000"));

            augrt_writelog(AUGRT_LOGINFO, "host: %s", host);
            augrt_writelog(AUGRT_LOGINFO, "serv: %s", serv);
            augrt_writelog(AUGRT_LOGINFO, "conns: %d", conns_);
            augrt_writelog(AUGRT_LOGINFO, "echos: %d", echos_);

            for (; estab_ < conns_; ++estab_)
                tcpconnect(host, serv, new state(echos_));
            return true;
        }
        void
        do_closed(const object& sock)
        {
            state& s(*sock.user<state>());
            pushxy(xy_, s.usecs_);
            delete &s;
            if (0 < --estab_) {
                augrt_writelog(AUGRT_LOGINFO, "%d established", estab_);
                return;
            }

            unsigned long end(ptimer_.now());
            double ms(end / 1000.0);

            augrt_writelog(AUGRT_LOGINFO, "total time: %f ms", ms);

            ms /= static_cast<double>(conns_);
            augrt_writelog(AUGRT_LOGINFO, "time per conn: %f ms", ms);

            ms /= static_cast<double>(echos_);
            augrt_writelog(AUGRT_LOGINFO, "echos per sec: %f", 1000.0 / ms);

            double k(static_cast<double>(bytes_) / 1024);
            augrt_writelog(AUGRT_LOGINFO, "total size: %f k", k);

            stopall();

            writexy(xy_);
        }
        void
        do_connected(object& sock, const char* addr, unsigned short port)
        {
            const char* sslctx = augrt::getenv("session.bench.sslcontext", 0);
            if (sslctx) {
                writelog(AUGRT_LOGINFO, "sslcontext: %s", sslctx);
                setsslclient(sock, sslctx);
            }

            state& s(*sock.user<state>());
            s.usecs_.push_back(ptimer_.now());
            send_(sock);
            --s.tosend_;
        }
        void
        do_data(const object& sock, const void* buf, size_t len)
        {
            bytes_ += len;
            state& s(*sock.user<state>());
            tokenise(static_cast<const char*>(buf),
                     static_cast<const char*>(buf) + len, s.tok_, '\n',
                     eachline(send_, sock, ptimer_, s.usecs_));
        }
        bool
        do_authcert(const object& sock, const char* subject,
                    const char* issuer)
        {
            augrt_writelog(AUGRT_LOGINFO, "checking subject...");
            return true;
        }
        ~benchsession() AUGRT_NOTHROW
        {
        }
        benchsession()
            : conns_(0),
              estab_(0),
              bytes_(0)
        {
        }
        static session_base*
        create(const char* sname)
        {
            return new benchsession();
        }
    };

    typedef basic_module<basic_factory<benchsession> > module;
}

AUGRT_MODULE(module::init, module::term)
