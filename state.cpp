#include "stdafx.h"
#include "state.h"
#include "core.h"

using namespace std;
using namespace std::filesystem;

void DebugPrint(const wstringstream &ss)
{ 
	const std::wstring& s = ss.str();
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
		const path root = get_module_file_path();
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

void CState::RunScript() const
{
	const path script_path = get_lua_file_path();
	auto script_file = script_path.u8string();
	const int error = luaL_loadfile(L, (const char*)script_file.c_str()) || lua_pcall(L, 0, 0, 0);
	if (error != 0)
	{
		wstringstream ss;
		ss << L"Error running script: " << lua_tostring(L, -1) << endl;
		DebugPrint(ss);
		lua_pop(L, 1);
	}
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

path CState::GetAppPath() const
{
	return "";
}

path CState::GetOnIconPath() const
{
	return "";
}

path CState::GetOffIconPath() const
{
	return "";
}

path CState::GetAppWorkDir() const
{
	return "";
}

bool CState::GetAppHide() const {
	return true;
}

wstring CState::GetAppArgs() const
{
	return L"";
}

std::list<std::wstring> CState::GetCustomEnvList() const {
	std::list<std::wstring> rv;
	return rv;
 
}