#include <stdlib.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct { 
    lua_State *lua;
} MOXILUA;
MOXILUA *moxiluanew (void);
