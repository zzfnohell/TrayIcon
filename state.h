#pragma once
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

class CState
{ 
public:
    CState();
    ~CState();

    void Initialize();

    std::wstring GetAppArgs() const;
    std::filesystem::path GetOnIconPath() const;
    std::filesystem::path GetOffIconPath() const;
    std::filesystem::path GetAppPath() const;
    std::filesystem::path GetAppWorkDir() const;
    bool GetAppHide() const;
    void  RunScript() const;
	std::list<std::wstring> GetCustomEnvList() const;
private:
    lua_State* L{ nullptr }; 
};
