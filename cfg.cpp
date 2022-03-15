#include "stdafx.h"
#include "cfg.h"

#define SECTION_UI L"UI"
#define SECTION_APP L"App"
#define KEY_UI_ONIMAGE L"OnImage"
#define KEY_UI_ONIMAGE_DEFAULT L"image.ico"

#define KEY_UI_OFFIMAGE L"OffImage"
#define KEY_UI_OFFIMAGE_DEFAULT L"image.ico"

#define KEY_APP_PATH L"Path"
#define KEY_APP_PATH_DEFAULT NULL

#define KEY_APP_ARGS L"Args"
#define KEY_APP_ARGS_DEFAULT L""

#define KEY_APP_WORKDIR_PATH L"WorkDir"
#define KEY_APP_WORKDIR_PATH_DEFAULT L"."


#define KEY_APP_HIDE L"Hide"
#define KEY_APP_HIDE_DEFAULT 1

constexpr  WCHAR SECTION_ENV[] = L"Env";
constexpr  WCHAR SECTION_ENV_PREFIX[] = L"Env.Prefix";

using namespace std;
using namespace std::filesystem;


path get_module_file_path()
{
	const unique_ptr<WCHAR[]> buf = make_unique<WCHAR[]>(MAX_PATH);
	const DWORD size = GetModuleFileName(NULL, buf.get(), MAX_PATH);
	assert(size > 0);

	path p{ wstring{ buf.get(), size} };
	return p;
}



path Canonicalize(const path& p)
{
	if (p.is_relative())
	{
		const path root = CIniConfig::GetModuleDirectory();
		path rv = absolute(root / p);
		return rv;
	}

	return p;
}

path CIniConfig::GetPathValueFromIni(LPCWSTR section, LPCWSTR key, LPCWSTR default_value)
{
	const path ini_path = GetIniPath();
	const unique_ptr<WCHAR[]> buf = make_unique<WCHAR[]>(MAX_PATH);
	const DWORD size = GetPrivateProfileString(
		section,
		key,
		default_value,
		buf.get(),
		MAX_PATH,
		ini_path.c_str());
	assert(size > 0 && size < MAX_PATH);

	path p{ wstring{ buf.get(), size} };
	path rv = Canonicalize(p);
	return rv;
}


wstring	 CIniConfig::GetStringValueFromIni(LPCWSTR section, LPCWSTR key, LPCWSTR default_value)
{
	constexpr int kSize = 4 * 1024;
	const path ini_path = GetIniPath();
	const unique_ptr<WCHAR[]> buf = make_unique<WCHAR[]>(kSize);
	const DWORD size = GetPrivateProfileString(
		section,
		key,
		default_value,
		buf.get(),
		kSize,
		ini_path.c_str());
	assert(size > 0 && size < kSize);
	wstring rv{ buf.get(),size };
	return rv;
}

CIniConfig::CIniConfig()
{
}

void CIniConfig::Initialize()
{

}

path CIniConfig::GetModuleDirectory()
{
	const path p = get_module_file_path();
	const path dir = p.parent_path();
	return dir;
}

path CIniConfig::GetIniPath()
{
	path p = get_module_file_path();
	p.replace_extension(L".ini");
	return p;
}

path CIniConfig::GetWorkDirPath()
{
	path rv = GetPathValueFromIni(SECTION_APP, KEY_APP_WORKDIR_PATH, KEY_APP_WORKDIR_PATH_DEFAULT);
	return rv;
}


path CIniConfig::GetOnIconPath()
{
	path rv = GetPathValueFromIni(SECTION_UI, KEY_UI_ONIMAGE, KEY_UI_ONIMAGE_DEFAULT);
	return rv;
}

path CIniConfig::GetOffIconPath()
{
	path rv = GetPathValueFromIni(SECTION_UI, KEY_UI_OFFIMAGE, KEY_UI_OFFIMAGE_DEFAULT);
	return rv;
}

path CIniConfig::GetAppPath()
{
	path rv = GetPathValueFromIni(SECTION_APP, KEY_APP_PATH, KEY_APP_PATH_DEFAULT);
	return rv;
}

wstring CIniConfig::GetAppArgs()
{
	wstring args = GetStringValueFromIni(SECTION_APP, KEY_APP_ARGS, KEY_APP_ARGS_DEFAULT);
	return args;
}

void CIniConfig::GetSectionList(LPCWSTR section, std::list<std::wstring>& env_list)
{
	constexpr int kBufSize = 4 * 1024;
	const path ini_path = GetIniPath();
	const wstring int_path_str = ini_path.wstring();
	const wchar_t* ini_path_ptr = int_path_str.c_str();
	const unique_ptr<WCHAR[]> buf = make_unique<WCHAR[]>(kBufSize);
	[[maybe_unused]] const int read_size = GetPrivateProfileSection(section, buf.get(), kBufSize, ini_path_ptr);
	assert(read_size <= kBufSize - 2);
	WCHAR* p = buf.get();
	while (*p) {
		size_t n = wcslen(p);
		wstring s{ p, n };
		env_list.emplace_back(s);
		p += n + 1;
	}
}
void CIniConfig::GetEnvList(std::list<std::wstring>& env_list)
{
	GetSectionList(SECTION_ENV, env_list);
}

void CIniConfig::GetEnvPrefixList(std::list<std::wstring>& env_list)
{
	GetSectionList(SECTION_ENV_PREFIX, env_list);
}

bool CIniConfig::GetAppHide()
{
	const path ini_path = GetIniPath();
	const UINT val = GetPrivateProfileInt(SECTION_APP, KEY_APP_HIDE, KEY_APP_HIDE_DEFAULT, ini_path.c_str());
	const bool rv = val != 0;
	return rv;
}