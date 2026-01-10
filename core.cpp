#include "stdafx.h"
#include "core.h"
#include "env.h"
#include "state.h"

using namespace std;

const char MetaTableName[] = "CoreMeta";
const char ModuleName[] = "core";


static int l_core_get_env(lua_State* L) {

	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CState* self = (CState*)(*meta);
	const char* n = luaL_checkstring(L, 2);   

	wstring wname = utf8_to_wstring(std::string{ n });
	auto it = self->env_map_.find(wname);
	if (it != self->env_map_.end())
	{
		const std::wstring& wval = it->second;
		std::string val = wstring_to_utf8(wval);
		lua_pushlstring(L, val.data(), val.size());
	}
	else
	{
		lua_pushnil(L);  // not found
	}
	return 1;
}

static int l_core_set_env(lua_State* L) {

	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CState* self = (CState*)(*meta);
	const char* n = luaL_checkstring(L, 2);      // first argument
	const char* v= luaL_checkstring(L, 3); // second argument
	wstring wname = utf8_to_wstring(std::string{ n });
	wstring wval = utf8_to_wstring(std::string{ v });
	self->env_map_[wname] = wval;
	return 0;  
}

static int l_core_output(lua_State* L)
{
	const char* n = luaL_checkstring(L, 2);
	OutputDebugStringA(n);
	return 0;
}

// system.stop()
static int l_core_stop(lua_State* L)
{
	printf("system stopped\n");
	return 0;
}

static int lua_open_core(lua_State* L)
{
	// 创建 metatable
	luaL_newmetatable(L, MetaTableName);

	static const luaL_Reg methods[] = {
		{"getenv", l_core_get_env},
		{"setenv", l_core_set_env},
		{"output", l_core_output},
		{NULL, NULL}
	};
	luaL_setfuncs(L, methods, 0);

	// mt.__index = mt
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	// 创建 userdata（单例）
	void** ud = (void**)lua_newuserdatauv(L, sizeof(void*), 1);
	*ud = GlobalState;

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
