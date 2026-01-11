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

	const std::wstring& GetAppArgs() const;
	const std::filesystem::path& GetOnIconPath() const;
	const	std::filesystem::path& GetOffIconPath() const;
	const std::filesystem::path& GetAppPath() const;
	const	std::filesystem::path& GetAppWorkDir() const;
	bool GetTrayHide() const;
	void  RunScript() const;
	std::list<std::wstring> GetCustomEnvList() const;

	bool tray_hide_{ true };
	std::wstring app_args_;
	std::filesystem::path app_path_;
	std::filesystem::path work_dir_;
	std::filesystem::path on_icon_path_;
	std::filesystem::path off_icon_path_;
	std::map<std::wstring, std::wstring> env_map_;
private:
	lua_State* L{ nullptr };


};

extern CState* GlobalState;