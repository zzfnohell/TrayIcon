#pragma once

#define INI_VALUE_BUFFER_SIZE 2048

class CIniConfig
{
public:
	CIniConfig();
	virtual ~CIniConfig() = default;

	void Initialize();

	static void GetModuleDirectory(WCHAR* val);
	static void GetOnIconPath(WCHAR* val);
	static void GetOffIconPath(WCHAR* val);
	static void GetAppPath(WCHAR* val);
	static void GetAppArgs(WCHAR* val, DWORD size);
	static void GetWorkDirPath(WCHAR* val);
	static void GetIniPath(WCHAR* val);
	static void GetEnv(WCHAR* val, DWORD size);
	static void GetEnvPrefix(WCHAR* val, DWORD size);
private:
	static void GetPathValueFromIni(LPCWSTR section, LPCWSTR key, LPCWSTR default_value, WCHAR* val);
	static void GetStringValueFromIni(LPCWSTR section, LPCWSTR key, LPCWSTR default_value, WCHAR* val, DWORD size);
};
