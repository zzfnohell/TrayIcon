#pragma once

class CConfig
{
public:
	CConfig();
	virtual ~CConfig() = default;

	void Initialize();

	[[nodiscard]] LPCWSTR GetModuleDirectory() const;
	[[nodiscard]] LPCWSTR GetOnIconPath() const;
	[[nodiscard]] LPCWSTR GetOffIconPath() const;
	[[nodiscard]] LPCWSTR GetAppPath() const;
	[[nodiscard]] LPCWSTR GetAppArgs() const;
	[[nodiscard]] LPCWSTR GetWorkDirPath() const;
private:
	WCHAR m_ModuleDirectory[MAX_PATH];
	WCHAR m_IniPath[MAX_PATH];
	WCHAR m_OnIconPath[MAX_PATH];
	WCHAR m_OffIconPath[MAX_PATH];
	WCHAR m_AppPath[MAX_PATH];
	WCHAR m_WorkDirPath[MAX_PATH];
	WCHAR m_AppArgs[MAX_PATH];
};
