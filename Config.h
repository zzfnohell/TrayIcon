#pragma once
 
class CConfig
{
public:
	CConfig();
	virtual ~CConfig();

	void Initialize();

	LPCWSTR GetModuleDirectory() const;
	LPCWSTR GetOnIconPath() const;
	LPCWSTR GetOffIconPath() const;
	LPCWSTR GetAppPath() const;
	LPCWSTR GetAppArgs() const;
private:
	WCHAR m_ModuleDirectory[MAX_PATH];
	WCHAR m_IniPath[MAX_PATH];
	WCHAR m_OnIconPath[MAX_PATH];
	WCHAR m_OffIconPath[MAX_PATH];
	WCHAR m_AppPath[MAX_PATH];
	WCHAR m_AppArgs[MAX_PATH];
};

