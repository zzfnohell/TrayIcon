#pragma once

#include <atlbase.h>
#include <atlpath.h>

class CConfig
{
    public:
        CConfig();
        virtual ~CConfig();

        void Initialize();

        const ATL::CPath *GetModuleDirectory() const;
        const ATL::CPath *GetIconPath() const;
        const ATL::CPath *GetAppPath() const;
        const ATL::CString *GetAppArgs() const;
    private:
        ATL::CPath m_ModuleDirectory;
        ATL::CPath m_IniPath;
        ATL::CPath m_IconPath;
        ATL::CPath m_AppPath;
        ATL::CString m_AppArgs;

        void Canonicalize(ATL::CPath *path);
};

