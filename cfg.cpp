#include "stdafx.h"
#include "cfg.h"

using namespace std;
using namespace std::filesystem;


path get_module_file_path()
{
    const unique_ptr<WCHAR[]> buf = make_unique<WCHAR[]>(MAX_PATH);
    const DWORD size = GetModuleFileName(nullptr, buf.get(), MAX_PATH);
    assert(size > 0);

    path p{wstring{buf.get(), size}};
    return p;
}


path canonicalize(const path& p)
{
    if (p.is_relative())
    {
        const path root = CConfig::GetModuleDirectory();
        path rv = absolute(root / p);
        return rv;
    }

    return p;
}


CConfig::CConfig()
{
}

void CConfig::Initialize()
{
}

path CConfig::GetModuleDirectory()
{
    const path p = get_module_file_path();
    const path dir = p.parent_path();
    return dir;
}

path CConfig::GetLuaFilePath()
{
    path p = get_module_file_path();
    p.replace_extension(L".lua");
    return p;
}

path CConfig::GetWorkDirPath()
{
    return L"";
}


path CConfig::GetOnIconPath()
{
    return L"";
}

path CConfig::GetOffIconPath()
{
    return L"";
}

path CConfig::GetAppPath()
{
    return L"";
}
