#include "stdafx.h"
#include "core.h"

#include <filesystem>
using namespace std;
using namespace std::filesystem;


path get_module_file_path()
{
    const unique_ptr<WCHAR[]> buf = make_unique<WCHAR[]>(MAX_PATH);
    const DWORD size = GetModuleFileName(nullptr, buf.get(), MAX_PATH);
    assert(size > 0);

    path p{wstring{buf.get(), size}};
    return p;
}

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
    void** ud = lua_newuserdata(L, sizeof(void*));
    *ud = nullptr;

    // 绑定 metatable
    luaL_getmetatable(L, "SystemMeta");
    lua_setmetatable(L, -2);

    // 返回 userdata 作为 module
    return 1;
}

static void load_libs(lua_State* L)
{
    luaL_requiref(L, "core", luaopen_system, 1);
}

void CLuaCore::StartEngine(const path& script_path)
{
    assert(!L);
    L = luaL_newstate();
    luaL_openlibs(L);
    load_libs(L);
    auto script_file = script_path.u8string();
    const int error = luaL_loadfile(L, (const char *)script_file.c_str()) || lua_pcall(L, 0, 0, 0);
    if (error != 0)
    {
        printf("Lua 执行错误: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

CLuaCore::CLuaCore()
{
}

CLuaCore::~CLuaCore()
{
    if (L)
    {
        lua_close(L);
    }
}
