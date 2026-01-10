#pragma once
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
};

#include <map>

void core_load_libs(lua_State* L);