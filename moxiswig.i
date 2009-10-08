%module moxiswig

%{
#include "memcached.h"
#include "cproxy.h"

extern
int luaopen_moxiswig(lua_State *);
%}

typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

%include "cproxy.h"
%include "memcached.h"
