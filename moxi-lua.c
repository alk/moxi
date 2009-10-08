/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_LUA_H
#include "moxi-lua.h"
#endif

#include "memcached.h"
#include "cproxy.h"
#include <conflate.h>

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

// from lua.c from lua 5.1 distribution
// lua's copyright notice follows
/* 
 * Copyright (C) 1994-2008 Lua.org, PUC-Rio.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}

struct conflate_lua_cb_state {
    lua_State *lua;
    int index;
};

static
enum conflate_mgmt_cb_result conflate_lua_cb(void *opaque,
                                             conflate_handle_t *handle,
                                             const char *cmd,
                                             bool direct,
                                             kvpair_t *pair,
                                             conflate_form_result *r,
                                             void *data)
{
    struct conflate_lua_cb_state *const state = data;
    lua_State *const lua = state->lua;

    lua_pushcfunction(lua, traceback);

    lua_rawgeti(lua, LUA_REGISTRYINDEX, state->index);

    int rv = lua_pcall(lua, 0, 0, -2);

    lua_remove(lua, -2);
    if (rv) {
        if (settings.verbose > 1) {
            fprintf(stderr, "Got exception from lua command: %s\n", lua_tostring(lua, -1));
        }
        lua_pop(lua, 1);
        return RV_ERROR;
    } else {
        return RV_OK;
    }
}

static
int register_conflate_callback(lua_State *lua)
{
    luaL_checkstring(lua, 1);
    luaL_checkstring(lua, 2);
    luaL_checkany(lua, 3);

    struct conflate_lua_cb_state *state = malloc(sizeof(struct conflate_lua_cb_state));
    if (!state)
        return luaL_error(lua, "malloc failed");

    state->lua = lua;
    state->index = luaL_ref(lua, LUA_REGISTRYINDEX);

    conflate_register_mgmt_cb(lua_tostring(lua, 1), lua_tostring(lua, 2),
                              conflate_lua_cb, state);
    return 0;
}

extern
int luaopen_moxiswig(lua_State *);

static const luaL_Reg moxilib[] = {
	{"__init", luaopen_moxiswig},
    {"register_conflate_callback", register_conflate_callback},
    {NULL, NULL}
};


void lua_inspect_value(lua_State *lua, int index);
void lua_inspect_global(lua_State *lua, const char *name);

void lua_inspect_value(lua_State *lua, int index)
{
    lua_pushcfunction(lua, traceback);
    lua_getglobal(lua, "inspect");
    lua_pushvalue(lua, index);
    int rv = lua_pcall(lua, 1, 0, -3);
    if (rv) {
        fprintf(stderr, "inspect failed: %s\n", lua_tostring(lua, -1));
        lua_pop(lua, 1);
    }
    lua_pop(lua, 1);
}

void lua_inspect_global(lua_State *lua, const char *name)
{
    lua_getglobal(lua, name);
    lua_inspect_value(lua, lua_gettop(lua));
    lua_pop(lua, 1);
}

void moxilua_init_proxy_main(proxy_main *m)
{
    lua_State *lua = luaL_newstate();
    luaL_openlibs(lua);
    lua_settop(lua, 0);
    m->luastate = lua;
    luaL_register(lua, "moxi", moxilib);
    lua_pushcfunction(lua, traceback);
    int status = luaL_loadfile(lua, "per_proxy_main.lua");
    if (status) {
        if (settings.verbose > 0) {
            fprintf(stderr, "failed to load per-proxy lua code. Status code: %d\n", status);
        }
        goto out;
    }
    status = lua_pcall(lua, 0, 0, -2);
    if (status) {
        if (settings.verbose > 0) {
            fprintf(stderr, "per-proxy lua configuration failed: %s", lua_tostring(lua, -1));
        }
        goto out;
    }
out:
    lua_settop(lua, 0);
}
