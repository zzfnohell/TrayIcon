#include "stdafx.h"
#include "core.h"

const char MetaTableName[] = "CoreMeta";
const char ModuleName[] = "core";

static int l_core_set_env(lua_State* L) {

	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CCore* self = (CCore*)(*meta);
	const char* n = luaL_checkstring(L, 2);      // first argument
	const char* v= luaL_checkstring(L, 3); // second argument

	printf("n=%s, s=%s\n", n, v);

	return 0; // number of return values to Lua
}

static int l_core_run(lua_State* L)
{
	printf("system stopped\n");
	return 0;
}

// system.stop()
static int l_core_stop(lua_State* L)
{
	printf("system stopped\n");
	return 0;
}
static int l_core_gc(lua_State* L)
{
	void** self = (void**)luaL_checkudata(L, 1, MetaTableName);

	if (*self) {
		// free or delete your resource
		// example if it was created with new:
		delete (CCore*)(*self);

		*self = NULL; // avoid double free
	}

	return 0;
}
static int lua_open_core(lua_State* L)
{
	// 创建 metatable
	luaL_newmetatable(L, MetaTableName);

	static const luaL_Reg methods[] = {
		{"setenv", l_core_set_env},
		 {"__gc",     l_core_gc},
		{NULL, NULL}
	};
	luaL_setfuncs(L, methods, 0);

	// mt.__index = mt
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	// 创建 userdata（单例）
	void** ud = (void**)lua_newuserdatauv(L, sizeof(void*), 1);
	*ud = new CCore();

	// 绑定 metatable
	luaL_getmetatable(L, MetaTableName);
	lua_setmetatable(L, -2);

	// 返回 userdata 作为 module
	return 1;
}

  void core_load_libs(lua_State* L)
{
	luaL_requiref(L, ModuleName, lua_open_core, 1);
}
