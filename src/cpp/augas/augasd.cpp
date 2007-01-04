/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGAS_BUILD

#include "augnetpp.hpp"
#include "augsrvpp.hpp"
#include "augsyspp.hpp"
#include "augutilpp.hpp"

#include "augas/buffer.hpp"
#include "augas/exception.hpp"
#include "augas/listener.hpp"
#include "augas/manager.hpp"
#include "augas/module.hpp"
#include "augas/options.hpp"
#include "augas/utility.hpp"

#if !defined(_WIN32)
# include "augconfig.h"
#else // _WIN32
# define PACKAGE_BUGREPORT "mark@emantic.co.uk"
#endif // _WIN32

#include <cassert>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory> // auto_ptr<>
#include <sstream>
#include <vector>

#include <time.h>

using namespace aug;
using namespace std;

namespace augas {

    typedef char cstring[AUG_PATH_MAX + 1];

    const char* program_;
    cstring conffile_= "";
    cstring rundir_ = "";
    cstring logdir_ = "";
    options options_;
    bool daemon_(false);
    bool stopping_(false);

    void
    openlog_()
    {
        tm tm;
        aug::gmtime(tm);
        stringstream ss;
        ss << "augasd-" << setfill('0')
           << setw(4) << tm.tm_year + 1900
           << setw(2) << tm.tm_mon + 1
           << setw(2) << tm.tm_mday;
        openlog(makepath(logdir_, ss.str().c_str(), "log").c_str());
    }

    void
    doreconf_()
    {
        const char* value;
        if ((value = options_.get("loglevel", 0))) {
            unsigned level(strtoui(value, 10));
            AUG_DEBUG2("setting log level: %d", level);
            aug_setloglevel(level);
        }

        // Other directories may be specified relative to the run directory.

        aug::chdir(rundir_);
        if (daemon_) {

            const char* logdir(options_.get("rundir", "."));
            aug::chdir(options_.get("logdir", "."));
            realpath(logdir_, getcwd().c_str(), sizeof(logdir_));

            // Re-opening the log file facilitates rolling.

            openlog_();
        }

        AUG_DEBUG2("log level: %d", aug_loglevel());
        AUG_DEBUG2("run directory: %s", rundir_);
    }

    typedef map<int, sessptr> pending;

    struct eventarg {
        string sname_;
        aug_var arg_;
        ~eventarg() AUG_NOTHROW
        {
            aug_freevar(&arg_);
        }
        eventarg(const string& sname, void* arg, void (*free)(void*))
            : sname_(sname)
        {
            aug_setvarp(&arg_, arg, free);
        }
    };

    struct state {

        conncb_base& cb_;
        aug::conns conns_;
        timers timers_;
        mplexer mplexer_;
        manager manager_;
        pending pending_;

        explicit
        state(conncb_base& cb)
            : cb_(cb)
        {
            AUG_DEBUG2("adding event pipe");
            insertconn(conns_, aug_eventin(), cb);
            setioeventmask(mplexer_, aug_eventin(), AUG_IOEVENTRD);
        }
    };

    auto_ptr<state> state_;
    const aug_driver* base_(0);

    int
    extclose_(int fd)
    {
        AUG_DEBUG2("clearing io event mask '%d'", fd);
        aug_setioeventmask(state_->mplexer_, fd, 0);
        return base_->close_(fd);
    }

    void
    setextdriver_(fdref ref, conncb_base& cb, unsigned short mask)
    {
        // Override close function.

        static aug_driver extended = { extclose_, 0, 0, 0, 0, 0 };
        if (!base_) {
            base_ = &getdriver(ref);
            extdriver(extended, *base_);
        }

        insertconn(state_->conns_, ref, cb);
        try {
            setioeventmask(state_->mplexer_, ref, mask);
            setdriver(ref, extended);
        } catch (...) {
            removeconn(state_->conns_, ref);
            throw;
        }
    }

    void
    timercb_(int id, const aug_var* arg, unsigned* ms, aug_timers* timers)
    {
        AUG_DEBUG2("custom timer expiry");

        pending::iterator it(state_->pending_.find(id));
        sessptr sess(it->second);
        sess->expire(id, aug_getvarp(arg), *ms);
        if (0 == *ms)
            state_->pending_.erase(it);
    }

    // Thread-safe.

    const char*
    error_()
    {
        return aug_errdesc;
    }

    void
    reconf_()
    {
        aug_event e = { AUG_EVENTRECONF, AUG_VARNULL };
        writeevent(aug_eventout(), e);
    }

    // Thread-safe.

    void
    writelog_(int level, const char* format, ...)
    {
        // Cannot throw.

        va_list args;
        va_start(args, format);
        aug_vwritelog(level, format, args);
        va_end(args);
    }

    // Thread-safe.

    void
    vwritelog_(int level, const char* format, va_list args)
    {
        // Cannot throw.

        aug_vwritelog(level, format, args);
    }

    // Thread-safe.

    int
    post_(const char* sname, int type, void* user, void (*free)(void*))
    {
        try {

            auto_ptr<eventarg> arg(new eventarg(sname, user, free));

            aug_event e;
            e.type_ = AUG_EVENTUSER + type;
            aug_setvarp(&e.arg_, arg.get(), 0);
            writeevent(aug_eventout(), e);
            arg.release();
            return 0;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    const char*
    getenv_(const char* name)
    {
        try {
            return options_.get(name, 0);
        } AUG_SETERRINFOCATCH;
        return 0;
    }

    int
    tcpconnect_(const char* sname, const char* host, const char* serv)
    {
        try {

            sessptr sess(state_->manager_.getsess(sname));

            // TODO: alter tcpconnect to support a non-blocking mode of
            // operation.

            endpoint ep(null);
            smartfd sfd(tcpconnect(host, serv, ep));

            inetaddr addr(null);
            AUG_DEBUG2("connected to host '%s', port '%d'",
                       inetntop(getinetaddr(ep, addr)).c_str(),
                       static_cast<int>(ntohs(port(ep))));

            setnodelay(sfd, true);
            setnonblock(sfd, true);
            setextdriver_(sfd, state_->cb_, AUG_IOEVENTRD);

            augas_id id = 0; // TODO: resolve GCC warning: 'id' might be used
                             // uninitialized in this function.
            id = aug_nextid();
            connptr cptr(new augas::conn(sess, sfd, id, state_->timers_));
            state_->manager_.insert(cptr);
            try {
                cptr->open(ep);
            } catch (...) {
                state_->manager_.erase(*cptr);
            }
            return (int)id;

        } AUG_SETERRINFOCATCH;
        return -1;
    }
    int
    tcplisten_(const char* sname, const char* host, const char* serv)
    {
        try {

            endpoint ep(null);
            smartfd sfd(tcplisten(host, serv, ep));
            setextdriver_(sfd, state_->cb_, AUG_IOEVENTRD);

            sessptr sess(state_->manager_.getsess(sname));

            augas_id id = 0; // TODO: resolve GCC warning: 'id' might be used
                             // uninitialized in this function.
            id = aug_nextid();
            listenerptr lptr(new augas::listener(sess, sfd, id));
            state_->manager_.insert(lptr);

            inetaddr addr(null);
            AUG_DEBUG2("listening on interface '%s', port '%d'",
                       inetntop(getinetaddr(ep, addr)).c_str(),
                       static_cast<int>(ntohs(port(ep))));
            return (int)id;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    settimer_(const char* sname, int id, unsigned ms, void* arg,
              void (*free)(void*))
    {
        try {

            var v(arg, free);
            id = aug_settimer(cptr(state_->timers_), id, ms, timercb_,
                              cptr(v));
            if (0 < id)
                state_->pending_[id] = state_->manager_.getsess(sname);
            return id;

        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    resettimer_(const char* sname, int tid, unsigned ms)
    {
        try {
            return aug_resettimer(cptr(state_->timers_), tid, ms);
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    canceltimer_(const char* sname, int tid)
    {
        try {
            int ret(aug_canceltimer(cptr(state_->timers_), tid));

            // Only erase if aug_canceltimer() returns true: may be in the
            // midst of a processtimers() call, in which case,
            // aug_canceltimer() will return false for the timer being
            // expired.

            if (0 < ret)
                state_->pending_.erase(tid);
            return ret;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    shutdown_(augas_id cid)
    {
        try {
            fileptr file(state_->manager_.getbyid(cid));
            connptr cptr(smartptr_cast<conn>(file));
            if (null != cptr)
                cptr->shutdown();
            else
                state_->manager_.erase(*file);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    send_(const char* sname, augas_id cid, const char* buf, size_t size,
          unsigned flags)
    {
        try {
            switch (flags) {
            case AUGAS_SNDALL:
                if (!state_->manager_.sendall(state_->mplexer_, cid, sname,
                                              buf, size))
                    throw error(__FILE__, __LINE__, EHOSTCALL,
                                "connection has been shutdown");
                break;
            case AUGAS_SNDSELF:
                if (!state_->manager_.sendself(state_->mplexer_, cid, buf,
                                               size))
                    throw error(__FILE__, __LINE__, EHOSTCALL,
                                "connection has been shutdown");
                break;
            case AUGAS_SNDOTHER:
                state_->manager_.sendother(state_->mplexer_, cid, sname, buf,
                                           size);
                break;
            default:
                throw error(__FILE__, __LINE__, EHOSTCALL, "invalid flags");
            }
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    setrwtimer_(augas_id cid, unsigned ms, unsigned flags)
    {
        try {
            connptr conn(smartptr_cast<conn>(state_->manager_.getbyid(cid)));
            if (null == conn)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection-id '%d' not found", cid);
            conn->setrwtimer(ms, flags);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    resetrwtimer_(augas_id cid, unsigned ms, unsigned flags)
    {
        try {
            connptr conn(smartptr_cast<conn>(state_->manager_.getbyid(cid)));
            if (null == conn)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection-id '%d' not found", cid);
            conn->resetrwtimer(ms, flags);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    int
    cancelrwtimer_(augas_id cid, unsigned flags)
    {
        try {
            connptr conn(smartptr_cast<conn>(state_->manager_.getbyid(cid)));
            if (null == conn)
                throw error(__FILE__, __LINE__, ESTATE,
                            "connection-id '%d' not found", cid);
            conn->cancelrwtimer(flags);
            return 0;
        } AUG_SETERRINFOCATCH;
        return -1;
    }

    const struct augas_host host_ = {
        error_,
        reconf_,
        writelog_,
        vwritelog_,
        post_,
        getenv_,
        tcpconnect_,
        tcplisten_,
        settimer_,
        resettimer_,
        canceltimer_,
        shutdown_,
        send_,
        setrwtimer_,
        resetrwtimer_,
        cancelrwtimer_
    };

    void
    load_(conncb_base& cb)
    {
        AUG_DEBUG2("loading sessions");
        state_->manager_.load(options_, host_);

        // Remove any timers allocated to sessions that could not be opened.

        pending::iterator it(state_->pending_.begin()),
            end(state_->pending_.end());
        while (it != end) {
            if (!it->second->isopen()) {
                aug_warn("cancelling timer associated with failed session");
                aug_canceltimer(cptr(state_->timers_), it->first);
                state_->pending_.erase(it++);
            } else
                ++it;
        }
    }

    void
    accept_(fdref ref, const sessptr& sess, conncb_base& conncb)
    {
        aug_endpoint ep;

        AUG_DEBUG2("accepting connection");

        smartfd sfd(null);
        try {

            sfd = accept(ref, ep);

        } catch (const errinfo_error& e) {

            if (aug_acceptlost()) {
                aug_warn("accept() failed: %s", e.what());
                return;
            }
            throw;
        }

        AUG_DEBUG2("initialising connection '%d'", sfd.get());

        setnodelay(sfd, true);
        setnonblock(sfd, true);
        setextdriver_(sfd, conncb, AUG_IOEVENTRD);

        augas_id id(aug_nextid());
        connptr conn(new augas::conn(sess, sfd, id, state_->timers_));
        state_->manager_.insert(conn);
        try {
            conn->open(ep);
        } catch (...) {
            state_->manager_.erase(*conn);
        }
    }

    bool
    readevent_(conncb_base& cb)
    {
        aug_event event;
        AUG_DEBUG2("reading event");

        switch (aug::readevent(aug_eventin(), event).type_) {
        case AUG_EVENTRECONF:
            AUG_DEBUG2("received AUG_EVENTRECONF");
            if (*conffile_) {
                AUG_DEBUG2("reading: %s", conffile_);
                options_.read(conffile_);
            }
            doreconf_();
            state_->manager_.reconf();
            break;
        case AUG_EVENTSTATUS:
            AUG_DEBUG2("received AUG_EVENTSTATUS");
            break;
        case AUG_EVENTSTOP:
            AUG_DEBUG2("received AUG_EVENTSTOP");
            stopping_ = true;
            state_->manager_.teardown();
            break;
        default:
            assert(AUG_VTPTR == event.arg_.type_);
            {
                auto_ptr<eventarg> arg(static_cast<
                                       eventarg*>(aug_getvarp(&event.arg_)));
                sessptr sess(state_->manager_.getsess(arg->sname_));
                if (sess->isopen())
                    sess->event(event.type_ - AUG_EVENTUSER,
                                aug_getvarp(&arg->arg_));
                else
                    aug_warn("event not delivered to failed session");
            }
        }
        aug_freevar(&event.arg_);
        return true;
    }

    class service : public conncb_base, public service_base {

        bool
        do_callback(int fd, aug_conns& conns)
        {
            if (!ioevents(state_->mplexer_, fd))
                return true;

            if (fd == aug_eventin())
                return readevent_(*this);

            fileptr file(state_->manager_.getbyfd(fd));
            connptr cptr(smartptr_cast<conn>(file));

            if (null != cptr) {

                if (!cptr->process(state_->mplexer_)) {
                    state_->manager_.erase(*file);
                    return false;
                }

            } else
                accept_(fd, file->sess(), *this);

            return true;
        }

        const char*
        do_getopt(enum aug_option opt)
        {
            switch (opt) {
            case AUG_OPTCONFFILE:
                return *conffile_ ? conffile_ : 0;
            case AUG_OPTEMAIL:
                return PACKAGE_BUGREPORT;
            case AUG_OPTLONGNAME:
                return "aug application server";
            case AUG_OPTPIDFILE:
                return options_.get("pidfile", "augasd.pid");
            case AUG_OPTPROGRAM:
                return program_;
            case AUG_OPTSHORTNAME:
                return "augasd";
            }
            return 0;
        }

        void
        do_readconf(const char* conffile, bool daemon)
        {
            // The conffile is optional, if specified it will be an absolute
            // path.

            if (conffile) {

                AUG_DEBUG2("reading: %s", conffile);
                options_.read(conffile);

                // Store the absolute path to service any reconf requests.

                aug_strlcpy(conffile_, conffile, sizeof(conffile_));
            }

            daemon_ = daemon;

            // Once set, the run directory should not change.

            const char* rundir(options_.get("rundir", 0));
            realpath(rundir_, rundir ? rundir : getcwd().c_str(),
                     sizeof(rundir_));

            doreconf_();
        }

        void
        do_init()
        {
            AUG_DEBUG2("initialising daemon process");

            setsrvlogger("augasd");

            auto_ptr<state> s(new state(*this));
            state_ = s;
            try {
                load_(*this);
            } catch (...) {

                // Ownership back to local.

                s = state_;
                throw;
            }
        }

        void
        do_run()
        {
            struct timercb : timercb_base {
                void
                do_callback(idref ref, unsigned& ms, aug_timers& timers)
                {
                    AUG_DEBUG2("re-opening log file");
                    openlog_();
                }
            } cb;
            timer reopen(state_->timers_);

            // Re-open log file every minute.

            if (daemon_)
                reopen.set(60000, cb);

            AUG_DEBUG2("running daemon process");

            int ret(!0);
            while (!stopping_ || !state_->manager_.empty()) {

                if (state_->timers_.empty()) {

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_)))
                        ;

                } else {

                    struct timeval tv;
                    processtimers(state_->timers_, 0 == ret, tv);

                    scoped_unblock unblock;
                    while (AUG_RETINTR == (ret = waitioevents(state_
                                                              ->mplexer_,
                                                              tv)))
                        ;
                }

                processconns(state_->conns_);
            }
        }

        void
        do_term()
        {
            AUG_DEBUG2("terminating daemon process");
            state_->manager_.clear();
            state_.reset();
        }
    };
}

int
main(int argc, char* argv[])
{
    using namespace augas;

    try {

        aug_errinfo errinfo;
        scoped_init init(errinfo);
        try {
            service serv;
            program_ = argv[0];

            blocksignals();
            aug_setloglevel(AUG_LOGINFO);
            return main(serv, argc, argv);

        } catch (const errinfo_error& e) {
            perrinfo(e, "aug::errorinfo_error");
        } catch (const exception& e) {
            aug_error("std::exception: %s", e.what());
        }

    } catch (const exception& e) {
        cerr << e.what() << endl;
    }
    return 1; // aug_main() does not return.
}
