#include "stdafx.h"
#include "state.h"
#include "core.h"
#include "msg.h"

using namespace std;
using namespace std::filesystem;

CState* kStatePtr;

LPWSTR format_state_error(const wstring& s)
{   
	const rsize_t word_count = s.size() + 1;
	wchar_t* p = new wchar_t[word_count];
	wcscpy_s(p, word_count, s.c_str());
	return p;
}

void DebugPrint(const std::wstring& s)
{
	OutputDebugString(s.c_str());
}

static path get_module_file_path()
{
	const unique_ptr<WCHAR[]> buf = make_unique<WCHAR[]>(MAX_PATH);
	const DWORD size = GetModuleFileName(nullptr, buf.get(), MAX_PATH);
	assert(size > 0);

	path p{ wstring{buf.get(), size} };
	return p;
}

static path get_module_directory()
{
	const path p = get_module_file_path();
	const path dir = p.parent_path();
	return dir;
}

static path get_lua_file_path()
{
	path p = get_module_file_path();
	p.replace_extension(L".lua");
	return p;
}

static path canonicalize(const path& p)
{
	if (p.is_relative())
	{
		const path root = get_module_directory();
		path rv = absolute(root / p);
		return rv;
	}

	return p;
}

void CState::Initialize() {
	assert(!L);
	L = luaL_newstate();
	luaL_openlibs(L);
	core_load_libs(L);
}

bool CState::RunScript() const
{
	const path script_path = get_lua_file_path();
	auto script_file = script_path.u8string();
	const int error = luaL_loadfile(L, (const char*)script_file.c_str()) || lua_pcall(L, 0, 0, 0);
	if (error != 0)
	{
		wstringstream ss;
		ss << L"Error running script: " << lua_tostring(L, -1) << endl;
		auto wmsg = ss.str();
		DebugPrint(wmsg);
		LPWSTR msg = format_state_error(wmsg);
		// ReSharper disable once CppAssignedValueIsNeverUsed
		BOOL rc = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(msg));
		assert(rc);
		lua_pop(L, 1);
		return false;
	}

	return true;
}

CState::CState()
{
}

CState::~CState()
{
	if (L)
	{
		lua_close(L);
		L = nullptr;
	}
}

const path& CState::GetAppPath() const
{
	return app_path_;
}

void  CState::SetAppPath(const std::filesystem::path& val)
{ 
	app_path_ = canonicalize(val);
}

const path& CState::GetOnIconPath() const
{
	return on_icon_path_;
}

void  CState::SetOnIconPath(const std::filesystem::path& val) {
	on_icon_path_ = canonicalize(val);
}

const path& CState::GetOffIconPath() const
{
	return off_icon_path_;
}

void  CState::SetOffIconPath(const std::filesystem::path& val)
{
	off_icon_path_ = canonicalize(val);
}

const path& CState::GetAppWorkDir() const
{
	return work_dir_;
}

void  CState::SetAppWorkDir(const std::filesystem::path& val)
{
	work_dir_ = canonicalize(val);
}

bool  CState::GetTrayHide() const {
	return tray_hide_;
}

void   CState::SetTrayHide(bool val)
{
	tray_hide_ = val;
}

const wstring& CState::GetAppArgs() const
{
	return app_args_;
}

void  CState::SetAppArgs(const std::wstring& val) {
	app_args_ = val;
}