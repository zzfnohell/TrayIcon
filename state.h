#pragma once
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <map>

class CState {
  public:
    CState();
    ~CState();

    void Initialize();

    const std::wstring& GetAppArgs() const;
    const std::filesystem::path& GetOnIconPath() const;
    const std::filesystem::path& GetOffIconPath() const;
    const std::filesystem::path& GetAppPath() const;
    const std::filesystem::path& GetAppWorkDir() const;

    void SetAppArgs(const std::wstring&);
    void SetOnIconPath(const std::filesystem::path& val);
    void SetOffIconPath(const std::filesystem::path& val);
    void SetAppPath(const std::filesystem::path& val);
    void SetAppWorkDir(const std::filesystem::path& val);

    bool GetAppHide() const;
    void SetAppHide(bool val);

    bool RunScript() const;

    void Reset();

    std::map<std::wstring, std::wstring> env_map_;

    HWND kDlg{nullptr};

  private:
    lua_State* L{nullptr};

    bool app_hide_{true};
    std::wstring app_args_;
    std::filesystem::path app_path_;
    std::filesystem::path work_dir_;
    std::filesystem::path on_icon_path_;
    std::filesystem::path off_icon_path_;
};

extern CState* kStatePtr;
