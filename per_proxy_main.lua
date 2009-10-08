function table_print (tt, indent, done)
  done = done or {}
  indent = indent or 0
  if type(tt) == "table" then
    local sb = {}
    for key, value in pairs (tt) do
      table.insert(sb, string.rep (" ", indent)) -- indent it
      if type (value) == "table" and not done [value] then
        done [value] = true
        table.insert(sb, "{\n");
        table.insert(sb, table_print (value, indent + 2, done))
        table.insert(sb, string.rep (" ", indent)) -- indent it
        table.insert(sb, "}\n");
      elseif "number" == type(key) then
        table.insert(sb, string.format("\"%s\"\n", tostring(value)))
      else
        table.insert(sb, string.format(
            "%s = \"%s\"\n", tostring (key), tostring(value)))
       end
    end
    return table.concat(sb)
  else
    return tt .. "\n"
  end
end

function inspect(val)
   function insp(tbl)
      if  "nil"       == type( tbl ) then
         return tostring(nil)
      elseif  "table" == type( tbl ) then
         return table_print(tbl)
      elseif  "string" == type( tbl ) then
         return tbl
      else
         return tostring(tbl)
      end
   end
   print(insp(val))
end

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
             moxi.__init("swig")
             moxi.swig = moxiswig
             moxi.register_conflate_callback("luacall", "test command implemented in lua",
                                             function ()
                                                print "test message"
                                                print("msec_current_time is " .. moxi.swig.msec_current_time)
                                                error "asd"
                                             end)
          end)
