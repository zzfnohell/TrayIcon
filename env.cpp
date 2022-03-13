#include "stdafx.h"
#include "iniconfig.h"
using namespace std;

constexpr WCHAR ENV_DELIMITER = L'=';
constexpr WCHAR ENV_DELIMITER_S[] = L"=";

inline int strlen_with_terminal(int v)
{
    return v + 1;
}

bool iequals(const std::wstring_view& a, const std::wstring_view& b) {
    return std::equal(a.begin(), a.end(),
        b.begin(), b.end(),
        [](char a, char b) {
            return tolower(a) == tolower(b);
        });
}

void prefix_env(list<wstring>& env_list, list<wstring>& config_list) {

}

optional<tuple<wstring, wstring>> try_match(const wstring& s) {
    constexpr int match_size = 3;
    constexpr int name_match_index = 1;
    constexpr int value_match_index = 2;
    static wregex rgx(L"(\\w+)=(.*)"); wsmatch matches;
    if (regex_search(s, matches, rgx)) {
        if (matches.size() == match_size) {
            return optional<tuple<wstring, wstring>> {
                {
                    matches[name_match_index].str(), matches[value_match_index].str()
                }
            };
        }
    }

    return nullopt;
}

void replace_env(list<wstring>& env_list, const list<wstring>& config_list) {

    for (wstring& s : env_list) {
        if (auto kvp = try_match(s)) {
            wstring name, value;
            tie(name, value) = *kvp;
            for (const wstring& cs : config_list) {
                if (auto ckvp = try_match(cs)) {
                    wstring cname, cvalue;
                    tie(cname, cvalue) = *ckvp;
                    if (iequals(cname, name)) {
                        s = name + L"=" + cvalue;
                    }
                }
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
    list<wstring> replace_list{};

    CIniConfig::GetEnvList(replace_list);
    parse_env(env_list);
    replace_env(env_list, replace_list);
    prefix_env(env_list, replace_list);
    return NULL;
}