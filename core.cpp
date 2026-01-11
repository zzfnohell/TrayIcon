#include "stdafx.h"
#include "core.h"
#include "env.h"
#include "state.h"

using namespace std;
using namespace std::filesystem;

const char MetaTableName[] = "CoreMeta";
const char ModuleName[] = "core";


static int l_core_get_env(lua_State* L) {

	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CState* self = (CState*)(*meta);
	const char* n = luaL_checkstring(L, 2);

	wstring wname = ansi_to_wstring(std::string{ n });
	auto it = self->env_map_.find(wname);
	if (it != self->env_map_.end())
	{
		const std::wstring& wval = it->second;
		std::string val = wstring_to_ansi(wval);
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
	const char* v = luaL_checkstring(L, 3); // second argument
	wstring wname = ansi_to_wstring(std::string{ n });
	wstring wval = ansi_to_wstring(std::string{ v });
	self->env_map_[wname] = wval;
	return 0;
}

static int l_core_output(lua_State* L)
{
	const char* n = luaL_checkstring(L, 2);
	OutputDebugStringA(n);
	return 0;
}


static int l_core_set_icon_on(lua_State* L)
{
	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CState* state = (CState*)(*meta);
	const char* s = luaL_checkstring(L, 2);
	state->SetOnIconPath(s);
	return 0;
}

static int l_core_set_icon_off(lua_State* L)
{
	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CState* state = (CState*)(*meta);
	const char* s = luaL_checkstring(L, 2);
	state->SetOffIconPath(s);
	return 0;
}

static int l_core_set_app_path(lua_State* L)
{
	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CState* state = (CState*)(*meta);
	const char* s = luaL_checkstring(L, 2);
	state->SetAppPath(s);
	return 0;
}

static int l_core_set_app_args(lua_State* L)
{
	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CState* state = (CState*)(*meta);
	const char* s = luaL_checkstring(L, 2);
	state->SetAppArgs(ansi_to_wstring(std::string{ s }));
	return 0;
}

static int l_core_set_work_dir(lua_State* L)
{
	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CState* state = (CState*)(*meta);
	const char* s = luaL_checkstring(L, 2);
	state->SetAppWorkDir(s);
	return 0;
}

static int l_core_set_tray_hide(lua_State* L)
{
	void** meta = (void**)luaL_checkudata(L, 1, MetaTableName);
	CState* state = (CState*)(*meta);
	luaL_checktype(L, 2, LUA_TBOOLEAN);
	int val = lua_toboolean(L, 2);
	state->SetTrayHide(val != 0);
	return 0;
}
static int lua_open_core(lua_State* L)
{
	// 创建 metatable
	luaL_newmetatable(L, MetaTableName);

	static const luaL_Reg methods[] = {
		{"getenv", l_core_get_env},
		{"setenv", l_core_set_env},
		{"seticonon", l_core_set_icon_on},
		{"seticonoff", l_core_set_icon_off},
		{"setapppath", l_core_set_app_path},
		{"setappargs", l_core_set_app_args},
		{"setappworkdir", l_core_set_work_dir},
		{"settrayhide", l_core_set_tray_hide},
		{"output", l_core_output},
		{NULL, NULL}
	};
	luaL_setfuncs(L, methods, 0);

	// mt.__index = mt
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	// 创建 userdata（单例）
	void** ud = (void**)lua_newuserdatauv(L, sizeof(void*), 1);
	*ud = kStatePtr;

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
