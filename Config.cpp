#include "stdafx.h"
#include "Config.h"

using namespace ATL;

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

CConfig::CConfig()
    : m_ModuleDirectory(),
      m_IniPath(),
      m_OnIconPath(),
      m_OffIconPath(),
      m_AppPath(),
      m_AppArgs()
{
}


CConfig::~CConfig()
{
}


void CConfig::Canonicalize(ATL::CPath *path)
{
    if (path)
    {
        if (path->IsRelative())
        {
            CPath tmp = this->m_ModuleDirectory;
            tmp.Append(*path);
            *path = tmp;
        }

        path->Canonicalize();
    }
}

void CConfig::Initialize()
{
    CPath::XCHAR path[MAX_PATH];
    WCHAR buffer[BUFFER_SIZE];
    DWORD size = GetModuleFileName(NULL, path, MAX_PATH);
    assert(size > 0);
    m_IniPath = path;
    m_IniPath.RemoveExtension();
    m_IniPath.AddExtension(L".ini");
    m_ModuleDirectory = path;
    m_ModuleDirectory.RemoveFileSpec();
    GetPrivateProfileString(
        SECTION_UI,
        KEY_UI_ONIMAGE,
        KEY_UI_ONIMAGE_DEFAULT,
        buffer,
        BUFFER_SIZE, m_IniPath);
    m_OnIconPath = buffer;
    Canonicalize(&m_OnIconPath);
    GetPrivateProfileString(
        SECTION_UI,
        KEY_UI_OFFIMAGE,
        KEY_UI_OFFIMAGE_DEFAULT,
        buffer,
        BUFFER_SIZE, m_IniPath);
    m_OffIconPath = buffer;
    Canonicalize(&m_OffIconPath);
    GetPrivateProfileString(
        SECTION_APP,
        KEY_APP_PATH,
        KEY_APP_PATH_DEFAULT,
        buffer,
        BUFFER_SIZE,
        m_IniPath);
    m_AppPath = buffer;
    GetPrivateProfileString(
        SECTION_APP,
        KEY_APP_ARGS,
        KEY_APP_ARGS_DEFAULT,
        buffer,
        BUFFER_SIZE,
        m_IniPath);
    m_AppArgs = buffer;
}

const ATL::CPath *CConfig::GetModuleDirectory() const\
{
    return &m_ModuleDirectory;
}

const ATL::CPath *CConfig::GetOnIconPath() const
{
    return &m_OnIconPath;
}

const ATL::CPath *CConfig::GetOffIconPath() const
{
    return &m_OffIconPath;
}

const ATL::CPath *CConfig::GetAppPath() const
{
    return &m_AppPath;
}

const ATL::CString *CConfig::GetAppArgs() const
{
    return &m_AppArgs;
}
