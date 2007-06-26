require 'log'

# post(), dispatch()

module RbTest2
    def RbTest2.stop
        Log.debug("stop()")
    end
    def RbTest2.start(sname)
        Log.debug("start(): #{sname}")
        Augrt.dispatch("group1", "foo", 101.to_s)
        Augrt.post(sname, "nil", nil)
    end 
    def RbTest2.event(frm, type, user)
        Log.debug("event(): #{user}")
        if type == "foo"
            if user.to_i != 101
                Log.error("unexpected user data")
            end
            Augrt.dispatch(frm, "bar", "202")
        elsif type == "bar"
            if user.to_i != 202
                Log.error("unexpected user data")
            end
        elsif type == "nil"
            if user != nil
                Log.error("unexpected user data")
            end
            Augrt.stopall
        else
            Log.error("unexpected type")
        end
    end
end