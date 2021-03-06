# Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>
#
# This file is part of Aug written by Mark Aylett.
#
# Aug is released under the GPL with the additional exemption that compiling,
# linking, and/or using OpenSSL is allowed.
#
# Aug is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# Aug is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

require 'log'

# post(), dispatch()

module RbTest2
    def self.stop
        Log.debug("stop()")
    end
    def self.start(sname)
        Log.debug("start(): #{sname}")
        AugRb.dispatch("topic1", "foo", 0, 101.to_s)
        AugRb.post(sname, "nil", 0, nil)
    end 
    def self.event(frm, type, id, ob)
        Log.debug("event(): #{ob}")
        if type == "foo"
            if ob.to_i != 101
                Log.error("unexpected ob data")
            end
            AugRb.dispatch(frm, "bar", 0, "202")
        elsif type == "bar"
            if ob.to_i != 202
                Log.error("unexpected ob data")
            end
        elsif type == "nil"
            if ob != nil
                Log.error("unexpected ob data")
            end
            AugRb.stopall
        else
            Log.error("unexpected type")
        end
    end
end
