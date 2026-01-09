#include "stdafx.h"
#include "core.h"
 
static int l_system_run(lua_State* L)
{
	printf("system running\n");
	return 0;
}

// system.stop()
static int l_system_stop(lua_State* L)
{
	printf("system stopped\n");
	return 0;
}

static int luaopen_system(lua_State* L)
{
	// 创建 metatable
	luaL_newmetatable(L, "SystemMeta");

	static const luaL_Reg methods[] = {
		{"run", l_system_run},
		{"stop", l_system_stop},
		{NULL, NULL}
	};
	luaL_setfuncs(L, methods, 0);

	// mt.__index = mt
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	// 创建 userdata（单例）
	void* ud = lua_newuserdatauv(L, sizeof(void*), 1);
	ud = nullptr;

	// 绑定 metatable
	luaL_getmetatable(L, "SystemMeta");
	lua_setmetatable(L, -2);

	// 返回 userdata 作为 module
	return 1;
}

  void load_libs(lua_State* L)
{
	luaL_requiref(L, "core", luaopen_system, 1);
}
