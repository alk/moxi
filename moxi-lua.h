#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_LUA_H
#include <lua.h>
MOXILUA *moxiluanew (void);

typedef struct {
    lua_State *lua;
} MOXILUA;
#endif
