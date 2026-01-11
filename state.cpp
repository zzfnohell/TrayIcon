#include "stdafx.h"
#include "core.h"
#include "msg.h"
#include "state.h"

using namespace std;
using namespace std::filesystem;

CState* kStatePtr;

LPWSTR format_state_error(const wstring& s) noexcept
{
    const rsize_t word_count = s.size() + 1;
    wchar_t* p = new wchar_t[word_count];
    wcscpy_s(p, word_count, s.c_str());
    return p;
}

inline void DebugPrint(const std::wstring& s) noexcept
{
    OutputDebugString(s.c_str());
}

inline void ShowError(HWND dlg, const wstring& wmsg) noexcept
{
    LPWSTR msg = format_state_error(wmsg);
    // ReSharper disable once CppAssignedValueIsNeverUsed
    BOOL rc = PostMessage(dlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(msg));
    assert(rc);
}

static path get_module_file_path() noexcept
{
    const unique_ptr<WCHAR[]> buf = make_unique<WCHAR[]>(MAX_PATH);
    const DWORD size = GetModuleFileName(nullptr, buf.get(), MAX_PATH);
    assert(size > 0);

    path p{wstring{buf.get(), size}};
    return p;
}

static path get_module_directory() noexcept
{
    const path p = get_module_file_path();
    const path dir = p.parent_path();
    return dir;
}

static path get_lua_file_path() noexcept
{
    path p = get_module_file_path();
    p.replace_extension(L".lua");
    return p;
}

static path canonicalize(const path& p) noexcept
{
    if (p.is_relative())
    {
        const path root = get_module_directory();
        path rv = absolute(root / p);
        return rv;
    }

    return p;
}

void CState::Initialize() noexcept
{
    assert(!L);
    L = luaL_newstate();
    luaL_openlibs(L);
    core_load_libs(L);
}

bool CState::RunScript() const noexcept
{
    const path script_path = get_lua_file_path();
    auto script_file = script_path.u8string();
    const int error = luaL_loadfile(L, (const char*)script_file.c_str()) || lua_pcall(L, 0, 0, 0);
    if (error != 0)
    {
        wstringstream ss;
        ss << L"Error running script: " << lua_tostring(L, -1) << endl;
        auto wmsg = ss.str();
        lua_pop(L, 1);

        DebugPrint(wmsg);

        ShowError(kDlg, wmsg);
        return false;
    }

    return true;
}

void CState::Reset() noexcept
{
    app_hide_ = true;
    app_args_.clear();
    app_path_.clear();
    work_dir_.clear();
    on_icon_path_.clear();
    off_icon_path_.clear();
    env_map_.clear();
}

CState::CState() noexcept
{
}

CState::~CState() noexcept
{
    if (L)
    {
        lua_close(L);
        L = nullptr;
    }
}

const path& CState::GetAppPath() const noexcept
{
    return app_path_;
}

void CState::SetAppPath(const std::filesystem::path& val) noexcept
{
    app_path_ = canonicalize(val);
}

const path& CState::GetOnIconPath() const noexcept
{
    return on_icon_path_;
}

void CState::SetOnIconPath(const std::filesystem::path& val) noexcept
{
    on_icon_path_ = canonicalize(val);
}

const path& CState::GetOffIconPath() const noexcept
{
    return off_icon_path_;
}

void CState::SetOffIconPath(const std::filesystem::path& val) noexcept
{
    off_icon_path_ = canonicalize(val);
}

const path& CState::GetAppWorkDir() const noexcept
{
    return work_dir_;
}

void CState::SetAppWorkDir(const std::filesystem::path& val) noexcept
{
    work_dir_ = canonicalize(val);
}

bool CState::GetAppHide() const noexcept
{
    return app_hide_;
}

void CState::SetAppHide(bool val) noexcept
{
    app_hide_ = val;
}

const wstring& CState::GetAppArgs() const noexcept
{
    return app_args_;
}

void CState::SetAppArgs(const std::wstring& val) noexcept
{
    app_args_ = val;
}
