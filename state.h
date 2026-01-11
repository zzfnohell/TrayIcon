#pragma once
extern "C"
{
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <map>

class CState
{
  public:
    CState() noexcept;
    ~CState() noexcept;

    void Initialize() noexcept;

    const std::wstring& GetAppArgs() const noexcept;
    const std::filesystem::path& GetOnIconPath() const noexcept;
    const std::filesystem::path& GetOffIconPath() const noexcept;
    const std::filesystem::path& GetAppPath() const noexcept;
    const std::filesystem::path& GetAppWorkDir() const noexcept;

    void SetAppArgs(const std::wstring&) noexcept;
    void SetOnIconPath(const std::filesystem::path& val) noexcept;
    void SetOffIconPath(const std::filesystem::path& val) noexcept;
    void SetAppPath(const std::filesystem::path& val) noexcept;
    void SetAppWorkDir(const std::filesystem::path& val) noexcept;

    bool GetAppHide() const noexcept;
    void SetAppHide(bool val) noexcept;

    bool RunScript() const noexcept;

    void Reset() noexcept;

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
