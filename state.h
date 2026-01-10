#pragma once
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <map>

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

	std::map<std::wstring, std::wstring> env_map_;
private:
	lua_State* L{ nullptr };

	bool hide_{ true };
	std::wstring args_;
	std::filesystem::path work_dir_;
	std::filesystem::path on_icon_path_;
	std::filesystem::path off_icon_path_;

};

extern CState* GlobalState;