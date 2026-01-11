#pragma once
extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
};

#include <map>

void core_load_libs(lua_State* L) noexcept;
