#include "stdafx.h"
#include "config.h"

#define BUFFER_SIZE 2048
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

CConfig::CConfig(): m_ModuleDirectory{0}, m_IniPath{0}, m_OnIconPath{0}, m_OffIconPath{0}, m_AppPath{0}, m_AppArgs{0}
{
}


CConfig::~CConfig()
{
}


void Canonicalize(WCHAR path[], WCHAR dir[])
{
	WCHAR tmp[MAX_PATH];
	if (PathIsRelative(path))
	{
		PathCombine(tmp, dir, path);
	}
	else
	{ 
		wcscpy_s(tmp, path);
	}

	PathCanonicalize(path, tmp);
}

void CConfig::Initialize()
{
	WCHAR buffer[BUFFER_SIZE];
	const DWORD size = GetModuleFileName(NULL, m_ModuleDirectory, MAX_PATH);
	assert(size > 0);
	wcscpy_s(m_IniPath, m_ModuleDirectory);

	PathRemoveExtension(m_IniPath);
	PathAddExtension(m_IniPath, L".ini");

	PathRemoveFileSpec(m_ModuleDirectory);

	GetPrivateProfileString(
		SECTION_UI,
		KEY_UI_ONIMAGE,
		KEY_UI_ONIMAGE_DEFAULT,
		buffer,
		BUFFER_SIZE, m_IniPath);

	wcscpy_s(m_OnIconPath, buffer);
	Canonicalize(m_OnIconPath, m_ModuleDirectory);

	GetPrivateProfileString(
		SECTION_UI,
		KEY_UI_OFFIMAGE,
		KEY_UI_OFFIMAGE_DEFAULT,
		buffer,
		BUFFER_SIZE, m_IniPath);

	wcscpy_s(m_OffIconPath, buffer);
	Canonicalize(m_OffIconPath, m_ModuleDirectory);

	GetPrivateProfileString(
		SECTION_APP,
		KEY_APP_PATH,
		KEY_APP_PATH_DEFAULT,
		buffer,
		BUFFER_SIZE,
		m_IniPath);

	wcscpy_s(m_AppPath, buffer);
	Canonicalize(m_AppPath, m_ModuleDirectory);

	GetPrivateProfileString(
		SECTION_APP,
		KEY_APP_ARGS,
		KEY_APP_ARGS_DEFAULT,
		buffer,
		BUFFER_SIZE,
		m_IniPath);

	wcscpy_s(m_AppArgs, buffer);
}

LPCWSTR CConfig::GetModuleDirectory() const
{
	return m_ModuleDirectory;
}

LPCWSTR CConfig::GetOnIconPath() const
{
	return m_OnIconPath;
}

LPCWSTR CConfig::GetOffIconPath() const
{
	return m_OffIconPath;
}

LPCWSTR CConfig::GetAppPath() const
{
	return m_AppPath;
}

LPCWSTR CConfig::GetAppArgs() const
{
	return m_AppArgs;
}
