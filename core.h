#pragma once
#include <filesystem>

class CCore
{
public:
    CCore();
    ~CCore();

    void Initialize();
    static std::wstring GetAppArgs();

private:
    lua_State* L{nullptr};
};
