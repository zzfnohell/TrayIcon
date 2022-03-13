#include "stdafx.h"
#include "iniconfig.h"
using namespace std;

constexpr WCHAR ENV_DELIMITER = L'=';
constexpr WCHAR ENV_DELIMITER_S[] = L"=";

inline int strlen_with_terminal(int v)
{
    return v + 1;
}



void replace_env(list<wstring>& env_list, list<wstring>& config_list) {
    wregex rgx(L"(\\w+)=(.*)");
    constexpr int match_size = 3;
    constexpr int name_match_index = 1;
    constexpr int value_match_index = 2;
    for (wstring& s : env_list) {
        wsmatch matches;
        if (regex_search(s, matches, rgx)) {
            if (matches.size() == match_size) {
                const wstring& name = matches[name_match_index].str();
                const wstring& value = matches[value_match_index].str();
            }
        }
    }
}

void parse_env(list<wstring>& env_list) {
    LPWCH  env = GetEnvironmentStrings();
    LPWCH p = env;
    while (*p) {
        size_t n = wcslen(p);
        wstring s{ p, n };
        env_list.emplace_back(s);
        p += n + 1;
    }

    BOOL rt = FreeEnvironmentStrings(env);
    assert(rt);
}

LPVOID build_env_block()
{
    list<wstring> env_list{};
    list<wstring> config_list{};
    CIniConfig::GetEnvList(config_list);
    parse_env(env_list);
    replace_env(env_list, config_list);
    return NULL;
}