#pragma once
#include <list>
#include <filesystem>
class CIniConfig
{
public:
	CIniConfig();
	virtual ~CIniConfig() = default;

	void Initialize();

	static std::filesystem::path GetModuleDirectory();
	static std::filesystem::path GetOnIconPath();
	static std::filesystem::path GetOffIconPath();
	static std::filesystem::path GetAppPath();
	static std::filesystem::path GetWorkDirPath();
	static std::filesystem::path GetIniPath();

	static std::wstring GetAppArgs();
	static void GetEnvList(std::list<std::wstring>& env_list);
	static void GetEnvPrefixList(std::list<std::wstring>& env_list);
	static bool GetAppHide();
private:
	static void GetSectionList(LPCWSTR section, std::list<std::wstring>& env_list);
	static std::filesystem::path GetPathValueFromIni(LPCWSTR section, LPCWSTR key, LPCWSTR default_value);
	static std::wstring GetStringValueFromIni(LPCWSTR section, LPCWSTR key, LPCWSTR default_value);
};
