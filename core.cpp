#include "stdafx.h"
#include "core.h"
 
static int l_system_set_env(lua_State* L) {

	void** meta = (void**)luaL_checkudata(L, 1, "SystemMeta");
	CCore* self = (CCore*)(*meta);
	const char* n = luaL_checkstring(L, 2);      // first argument
	const char* v= luaL_checkstring(L, 3); // second argument

	printf("n=%s, s=%s\n", n, v);

	return 0; // number of return values to Lua
}

static int l_system_run(lua_State* L)
{
	printf("system stopped\n");
	return 0;
}

// system.stop()
static int l_system_stop(lua_State* L)
{
	printf("system stopped\n");
	return 0;
}
static int l_system_gc(lua_State* L)
{
	void** self = (void**)luaL_checkudata(L, 1, "SystemMeta");

	if (*self) {
		// free or delete your resource
		// example if it was created with new:
		delete (CCore*)(*self);

		*self = NULL; // avoid double free
	}

	return 0;
}
static int luaopen_system(lua_State* L)
{
	// 创建 metatable
	luaL_newmetatable(L, "SystemMeta");

	static const luaL_Reg methods[] = {
		{"run", l_system_run},
		{"stop", l_system_stop},
		{"setenv", l_system_set_env},
		 {"__gc",     l_system_gc},
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
	luaL_getmetatable(L, "SystemMeta");
	lua_setmetatable(L, -2);

	// 返回 userdata 作为 module
	return 1;
}

  void core_load_libs(lua_State* L)
{
	luaL_requiref(L, "core", luaopen_system, 1);
}
