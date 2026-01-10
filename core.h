#pragma once
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
};

#include <map>

class CCore {
public:

	std::wstring GetEnv(const std::wstring& name);
	void SetEnv(const std::wstring& name, const std::wstring& val);
private:
	std::map<std::wstring, std::wstring> env_map_;
};
void core_load_libs(lua_State* L);