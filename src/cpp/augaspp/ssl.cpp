/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGRTPP_BUILD
#include "augaspp/ssl.hpp"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if ENABLE_SSL

# include "augaspp/conn.hpp"

# include <openssl/err.h>
# include <openssl/ssl.h>

using namespace aug;

AUGRTPP_API
sslctx::~sslctx() AUG_NOTHROW
{
    SSL_CTX_free(ctx_);
}

AUGRTPP_API
sslctx::sslctx()
    : ctx_(SSL_CTX_new(SSLv23_method()))
{
    if (!ctx_)
        throw ssl_error(__FILE__, __LINE__, ERR_get_error());
}

AUGRTPP_API void
aug::initssl()
{
    // Global system initialization.

    SSL_library_init();
    SSL_load_error_strings();
}

// AUGRTPP_API void
// aug::setsslclient(conn_base& conn, sslctx& ctx)
// {
//     SSL* ssl = SSL_new(ctx);
//     BIO* sbio = BIO_new_socket((int)conn.sd().get(), BIO_NOCLOSE);
//     SSL_set_bio(ssl, sbio, sbio);
//     SSL_set_app_data(ssl, &conn);
//     aug_setsslclient(conn.sd().get(), ssl);
// }

// AUGRTPP_API void
// aug::setsslserver(conn_base& conn, sslctx& ctx)
// {
//     SSL* ssl = SSL_new(ctx);
//     BIO* sbio = BIO_new_socket((int)conn.sd().get(), BIO_NOCLOSE);
//     SSL_set_bio(ssl, sbio, sbio);
//     SSL_set_app_data(ssl, &conn);
//     aug_setsslserver(conn.sd().get(), ssl);
// }

#endif // ENABLE_SSL
