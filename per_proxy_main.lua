local function debugcall(thunk)
   local function errhandler(error)
      print(string.format("Error: %s, at %s\n", error, debug.traceback()))
      return error
   end
   local rv = {xpcall(thunk, errhandler)}
   if rv[1] then
      return unpack(rv, 2)
   else
      return error(rv[2])
   end
end

local function fbind (f, ...)
   local args = {...}
   return function (...)
             local newargs = {...}
             return debugcall(function ()
                                 local len = #args
                                 local callargs
                                 local newlen = #newargs

                                 if newlen == 0 then
                                    callargs = args
                                 else
                                    callargs = {unpack(args)}
                                    for i = len + 1, len + 1 + newlen do
                                       callargs[i] = newargs[i - len]
                                    end
                                 end

                                 return f(unpack(callargs))
                              end)
          end
end

-- debug.sethook(function (type, arg)
--                  local line = 'none'
--                  if type == 'line' then
--                     line = arg
--                  end
--                  print(string.format("-- trace:%s:%s", type, line))
--               end, "crl")

debugcall(function ()
             moxi.swig = moxi.__init("swig")
             moxi.register_conflate_callback("luacall", "test command implemented in lua",
                                             function ()
                                                print "test message"
                                                print("msec_current_time is " .. moxi.swig.msec_current_time)
                                                error "asd"
                                             end)
          end)
