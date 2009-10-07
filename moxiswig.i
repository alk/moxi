%module moxiswig

%{
#include "memcached.h"
#include "cproxy.h"

extern
int luaopen_moxiswig(lua_State *);
%}

%include "cproxy.h"
%include "memcached.h"
