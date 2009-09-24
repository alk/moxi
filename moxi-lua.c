#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_LUA_H
#include "moxi-lua.h"
#endif

#ifdef HAVE_LUA_H
MOXILUA* moxiluanew(void) {
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
#endif

/*int main(void) {
  int result;
  MOXILUA *moxilua= moxiluanew();
  result = lua_pcall(moxilua->lua, 0, LUA_MULTRET, 0);
  if (result) {
    fprintf(stderr, "Failed to run script: %s\n", lua_tostring(moxilua->lua, -1));
    exit(1);
  }

  exit(0);
}
*/
