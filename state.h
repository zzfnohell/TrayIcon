#pragma once
 

class CState
{ 
public:
    CState();
    ~CState();

    void Initialize();

    std::wstring GetAppArgs() const;
    std::filesystem::path GetOnIconPath() const;
    std::filesystem::path GetOffIconPath() const;
    std::filesystem::path GetAppPath() const;
    std::filesystem::path GetAppWorkDir() const;
    bool GetAppHide() const;
    void  RunScript() const;
private:
    lua_State* L{ nullptr };
    std::filesystem::path  script_path_{};
};
