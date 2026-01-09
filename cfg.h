#pragma once
#include <filesystem>

class CConfig
{
public:
    CConfig();
    virtual ~CConfig() = default;

    void Initialize();

    static std::filesystem::path GetModuleDirectory();
    static std::filesystem::path GetOnIconPath();
    static std::filesystem::path GetOffIconPath();
    static std::filesystem::path GetAppPath();
    static std::filesystem::path GetWorkDirPath();
    static std::filesystem::path GetLuaFilePath();
};
