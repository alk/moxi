#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_LUA_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct {
    lua_State *lua;
} MOXILUA;

MOXILUA* moxiluanew(void);

struct proxy_main;
void moxilua_init_proxy_main(struct proxy_main *m);

#endif
