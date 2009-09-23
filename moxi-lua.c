#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct { 
    lua_State *lua;
} MOXILUA;

MOXILUA *moxiluanew (void) {
  int status;
  MOXILUA *moxilua;
  lua_State *lua = luaL_newstate();
  luaL_openlibs(lua);
  lua_settop(lua, 0);
  /* Load the file containing the script we are going to run */
  status = luaL_loadfile(lua, "moxi.lua");
  if (status) {
    /* If something went wrong, error message is at the top of */
    /* the stack */
    fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(lua, -1));
    exit(1);
  }
  moxilua = malloc(sizeof(*moxilua));
  moxilua->lua = lua;
  return moxilua;
}
