#include "stdafx.h"
#include "iniconfig.h"

#define SECTION_UI L"UI"
#define SECTION_APP L"App"
#define KEY_UI_ONIMAGE L"OnImage"
#define KEY_UI_ONIMAGE_DEFAULT L"image.ico"

#define KEY_UI_OFFIMAGE L"OffImage"
#define KEY_UI_OFFIMAGE_DEFAULT L"image.ico"

#define KEY_APP_PATH L"Path"
#define KEY_APP_PATH_DEFAULT NULL
#define KEY_APP_ARGS L"Args"
#define KEY_APP_ARGS_DEFAULT NULL
#define KEY_APP_WORKDIR_PATH L"WorkDir"

constexpr  WCHAR SECTION_ENV[] = L"Env";
constexpr  WCHAR SECTION_ENV_PREFIX[] = L"Env.Prefix";

void Canonicalize(WCHAR path[], WCHAR dir[])
{
	WCHAR tmp[MAX_PATH] = { 0 };
	if (PathIsRelative(path))
	{
		PathCombine(tmp, dir, path);
	}
	else
	{
		wcscpy_s(tmp, path);
	}

	[[maybe_unused]] const auto suc = PathCanonicalize(path, tmp);
	assert(suc);
}

void CIniConfig::GetPathValueFromIni(LPCWSTR section, LPCWSTR key, LPCWSTR default_value, WCHAR* val)
{
	WCHAR ini_path[MAX_PATH];
	WCHAR mod_dir[MAX_PATH];

	GetModuleDirectory(mod_dir);
	GetIniPath(ini_path);

	GetPrivateProfileString(
		section,
		key,
		default_value,
		val,
		INI_VALUE_BUFFER_SIZE,
		ini_path);

	Canonicalize(val, mod_dir);
}


void CIniConfig::GetStringValueFromIni(LPCWSTR section, LPCWSTR key, LPCWSTR default_value, WCHAR* val, DWORD size)
{
	WCHAR ini_path[MAX_PATH];
	WCHAR mod_dir[MAX_PATH];

	GetModuleDirectory(mod_dir);
	GetIniPath(ini_path);

	GetPrivateProfileString(
		section,
		key,
		default_value,
		val,
		size,
		ini_path);
}

CIniConfig::CIniConfig()
{
}

void CIniConfig::Initialize()
{

}

void CIniConfig::GetModuleDirectory(WCHAR* val)
{
	[[maybe_unused]] const DWORD size = GetModuleFileName(NULL, val, MAX_PATH);
	assert(size > 0);

	[[maybe_unused]] const BOOL rc = PathRemoveFileSpec(val);
	assert(rc != 0);
}

void CIniConfig::GetIniPath(WCHAR* val)
{
	[[maybe_unused]] const DWORD size = GetModuleFileName(NULL, val, MAX_PATH);
	assert(size > 0);

	PathRemoveExtension(val);

	[[maybe_unused]] const BOOL rc = PathAddExtension(val, L".ini");
	assert(rc != 0);
}

void CIniConfig::GetOnIconPath(WCHAR* val)
{
	GetPathValueFromIni(SECTION_UI, KEY_UI_ONIMAGE, KEY_UI_ONIMAGE_DEFAULT, val);
}

void CIniConfig::GetOffIconPath(WCHAR* val)
{
	GetPathValueFromIni(SECTION_UI, KEY_UI_OFFIMAGE, KEY_UI_OFFIMAGE_DEFAULT, val);
}

void CIniConfig::GetAppPath(WCHAR* val)
{
	GetPathValueFromIni(SECTION_APP, KEY_APP_PATH, KEY_APP_PATH_DEFAULT, val);
}

void CIniConfig::GetAppArgs(WCHAR* val, DWORD size)
{
	GetStringValueFromIni(SECTION_APP, KEY_APP_ARGS, KEY_APP_ARGS_DEFAULT, val, size);
}

void CIniConfig::GetWorkDirPath(WCHAR* val)
{
	GetPathValueFromIni(SECTION_APP, KEY_APP_WORKDIR_PATH, NULL, val);
}

void CIniConfig::GetEnv (WCHAR* val, DWORD size)
{
	WCHAR ini_path[MAX_PATH];
	WCHAR mod_dir[MAX_PATH];

	GetModuleDirectory(mod_dir);
	GetIniPath(ini_path);

	GetPrivateProfileSection(SECTION_ENV, val, size, ini_path);
}

void CIniConfig::GetEnvPrefix(WCHAR* val, DWORD size)
{
	WCHAR ini_path[MAX_PATH];
	WCHAR mod_dir[MAX_PATH];

	GetModuleDirectory(mod_dir);
	GetIniPath(ini_path);

	GetPrivateProfileSection(SECTION_ENV_PREFIX, val, size, ini_path);
}

