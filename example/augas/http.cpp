#include "augaspp.hpp"
#include "augnetpp.hpp"
#include "augutilpp.hpp"
#include "augmarpp.hpp"
#include "augsyspp.hpp"

#include <fstream>
#include <sstream>

#include <sys/stat.h>

#if !defined(_WIN32)
# define _stat stat
# define _S_ISDIR S_ISDIR
# define _S_ISREG S_ISREG
#endif // !_WIN32

using namespace aug;
using namespace augas;
using namespace std;

namespace {

    bool
    stat(const char* path, struct _stat& sb)
    {
        if (-1 == _stat(path, &sb)) {
            if (ENOENT != errno)
                throw posix_error(__FILE__, __LINE__, errno);
            return false;
        }
        return true;
    }

    string
    utcdate()
    {
        char buf[64];
        time_t t;
        time(&t);
        strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
        return buf;
    }

    struct request {
        string method_, uri_, version_;
    };

    struct uri {
        string base_, path_, query_;
    };

    request&
    splitrequest(request& r, const string& s)
    {
        string::size_type first, last;
        if (string::npos == (first = s.find_first_of(' '))
            || string::npos == (last = s.find_last_of(' ')))
            throw error("malformed request");
        r.method_ = s.substr(0, first++);
        r.uri_ = s.substr(first, last - first);
        r.version_ = s.substr(last + 1);
        return r;
    }

    uri&
    splituri(uri& u, const string& s)
    {
        string::size_type pos;
        if (0 == s.compare(0, (pos = 7), "http://")
            || 0 == s.compare(0, (pos = 8), "https://")) {

            if (string::npos == (pos = s.find_first_of('/', pos))) {
                u.base_ = s;
                return u;
            }

            u.base_ = s.substr(0, pos++);
            u.path_ = s.substr(pos);

        } else
            u.path_ = s;

        if (string::npos != (pos = u.path_.find_last_of('?'))) {
            u.query_ = u.path_.substr(pos + 1);
            u.path_.erase(pos);
        }

        return u;
    }

    class nodes {
        vector<string>* xs_;
    public:
        explicit
        nodes(vector<string>& xs)
            : xs_(&xs)
        {
        }
        void
        operator ()(string& x)
        {
            if (x == "..") {
                if (xs_->empty())
                    throw error("bad path");
                xs_->pop_back();
            } else if (!x.empty() && x != ".")
                xs_->push_back(x);
        }
    };

    vector<string>
    splitpath(const string& path)
    {
        vector<string> v;
        splitn(path.begin(), path.end(), '/', nodes(v));
        return v;
    }

    string
    joinpath(const char* root, const vector<string>& nodes)
    {
        string path(root);
        vector<string>::const_iterator it(nodes.begin()), end(nodes.end());
        for (; it != end; ++it) {
            path += '/';
            path += *it;
        }
        return path;
    }

    class filecontent {
        smartfd sfd_;
        mmap mmap_;
    public:
        typedef filecontent arg_type;
        explicit
        filecontent(const char* path)
            : sfd_(aug::open(path, O_RDONLY)),
              mmap_(sfd_, 0, 0, AUG_MMAPRD)
        {
        }
        void*
        addr() const
        {
            return mmap_.addr();
        }
        size_t
        size() const
        {
            return mmap_.len();
        }
        static void
        destroy(arg_type* arg)
        {
            delete arg;
        }
        static const void*
        buf(arg_type& arg, size_t& size)
        {
            size = arg.size();
            return arg.addr();
        }
        static const void*
        buf(arg_type& arg)
        {
            return arg.addr();
        }
    };

    struct handler : basic_marhandler {
        static void
        message(const aug_var& var, const char* initial, aug_mar_t mar)
        {
            static const char ROOT[] = ".";

            augas_id id(aug_ptoi(var.arg_));

            aug_info("%s", initial);

            request r;
            splitrequest(r, initial);
            aug_info("method: [%s]", r.method_.c_str());
            aug_info("uri: [%s]", r.uri_.c_str());
            aug_info("version: [%s]", r.version_.c_str());

            uri u;
            splituri(u, r.uri_);

            if (r.method_ == "POST") {
                unsigned size;
                const char* query(static_cast<
                                  const char*>(content(mar, size)));
                u.query_ = string(query, size);
            }

            aug_info("base: [%s]", u.base_.c_str());
            aug_info("path: [%s]", u.path_.c_str());
            aug_info("query: [%s]", u.query_.c_str());

            header header(mar);
            header::const_iterator it(header.begin()),
                end(header.end());
            for (; it != end; ++it)
                aug_info("%s: %s", *it, header.getfield(it));

            u.path_ = urldecode(u.path_.begin(), u.path_.end());

            vector<string> nodes(splitpath(u.path_));
            string path(joinpath(ROOT, nodes));
            aug_info("path [%s]", path.c_str());

            stringstream ss;

            if (2 == nodes.size() && nodes[0] == "services") {

                dispatch(nodes[1].c_str(),
                         "application/x-www-form-urlencoded",
                         u.query_.c_str(), u.query_.size());

                unsigned size;
                const char* value(static_cast<const char*>
                                  (getfield(mar, "Host", size)));

                ss << "HTTP/1.1 303 See Other\r\n"
                   << "Date: " << utcdate() << "\r\n"
                   << "Location: http://" << value << "\r\n"
                   << "Content-Length: 0\r\n"
                   << "\r\n";

                send(id, ss.str().c_str(), ss.str().size());

            } else {

                for (;;) {
                    struct _stat sb;
                    if (!stat(path.c_str(), sb))
                        throw runtime_error("file does not exist");

                    if (_S_ISDIR(sb.st_mode)) {
                        path += "/index.html";
                        continue;
                    } else if (!_S_ISREG(sb.st_mode))
                        throw runtime_error("invalid file type");
                    break;
                }

                auto_ptr<filecontent> ptr(new filecontent(path.c_str()));
                ss << "HTTP/1.1 200 OK\r\n"
                   << "Date: " << utcdate() << "\r\n"
                   << "Content-Type: text/html\r\n"
                   << "Content-Length: " << ptr->size() << "\r\n"
                   << "\r\n";

                send(id, ss.str().c_str(), ss.str().size());
                aug_var var;
                bindvar<filecontent>(var, *ptr);
                augas_var v2;
                memcpy(&v2, &var, sizeof(v2));
                sendv(id, v2);
                ptr.release();
            }


            unsigned size;
            const char* value(static_cast<const char*>
                              (getfield(mar, "Connection", size)));
            if (value && size && aug_strcasestr(value, "close")) {
                aug_info("closing");
                shutdown(id);
            }
        }
    };

    struct httpserv : basic_serv {
        bool
        do_start(const char* sname)
        {
            aug_info("starting...");
            const char* serv = augas::getenv("service.http.serv");
            if (!serv)
                return false;

            tcplisten("0.0.0.0", serv);
            return true;
        }
        void
        do_closed(const object& sock)
        {
            aug_info("closed");
            if (sock.user()) {
                auto_ptr<marparser> parser(sock.user<marparser>());
                endmar(*parser);
            }
        }
        void
        do_teardown(const object& sock)
        {
            shutdown(sock);
        }
        bool
        do_accept(object& sock, const char* addr, unsigned short port)
        {
            aug_var var = { 0, aug_itop(sock.id()) };
            sock.setuser(new marparser(0, marhandler<handler>(), var));
            setrwtimer(sock, 30000, AUGAS_TIMRD);
            return true;
        }
        void
        do_data(const object& sock, const void* buf, size_t size)
        {
            marparser& parser(*sock.user<marparser>());
            try {
                parsemar(parser, static_cast<const char*>(buf),
                         static_cast<unsigned>(size));
            } catch (...) {
                shutdown(sock);
                throw;
            }
        }
        void
        do_rdexpire(const object& sock, unsigned& ms)
        {
            aug_info("no data received for 30 seconds");
            shutdown(sock);
        }
        static serv_base*
        create(const char* sname)
        {
            return new httpserv();
        }
    };

    typedef basic_module<basic_factory<httpserv> > module;
}

AUGAS_MODULE(module::init, module::term)
