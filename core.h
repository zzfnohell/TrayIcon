#pragma once
#include <filesystem>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

void core_load_libs(lua_State* L);