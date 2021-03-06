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
#ifndef AUGSERVPP_MAIN_HPP
#define AUGSERVPP_MAIN_HPP

#include "augservpp/config.hpp"

#include "augserv/main.h"

namespace aug {

    /**
     * On Windows, the Service Manager calls the service entry point on a
     * separate thread - automatic variables on the main thread's stack will
     * not be visible from the service thread.  A shallow copy of the service
     * structure is, therefore, performed by aug_main().
     */

    inline int
    main(int argc, char* argv[], const aug_serv& serv)
    {
        return aug_main(argc, argv, &serv);
    }
}

#endif // AUGSERVPP_MAIN_HPP
