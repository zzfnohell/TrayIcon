#include "stdafx.h"
#include "iniconfig.h"
using namespace std;

constexpr WCHAR ENV_DELIMITER = L'=';
constexpr WCHAR ENV_DELIMITER_S[] = L"=";

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
bool iequals(const std::wstring_view& a, const std::wstring_view& b) {
    return std::equal(a.begin(), a.end(),
        b.begin(), b.end(),
        [](char a, char b) {
            return tolower(a) == tolower(b);
        });
}

void prefix_env(list<wstring>& env_list, list<wstring>& config_list) {

    for (wstring& s : env_list) {
        if (auto kvp = try_match(s)) {
            wstring name, value;
            tie(name, value) = *kvp;
            wstring new_val = value;
            bool has_prefix = false;
            for (const wstring& cs : config_list) {
                if (auto ckvp = try_match(cs)) {
                    wstring cname, cvalue;
                    tie(cname, cvalue) = *ckvp;
                    if (iequals(cname, name)) {
                        new_val = cvalue + new_val;
                        has_prefix = true;
                    }
                }
            }

            if (has_prefix) {
                s = name + L"=" + new_val;
            }
        }
    }
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

    for (const wstring& cs : config_list) {
        if (auto ckvp = try_match(cs)) {
            bool append = true;
            wstring cname;
            tie(cname, ignore) = *ckvp;
            for (wstring& s : env_list) {
                if (auto kvp = try_match(s)) {
                    wstring name;
                    tie(name, ignore) = *kvp;
                    if (iequals(name, cname)) {
                        append = false;
                        break;
                    }
                }
            }

            if (append) {
                env_list.push_back(cs);
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

unique_ptr<wchar_t[]> env_list_to_block(const list<wstring>& env_list) {
    size_t total = 0;
    for (const wstring& s : env_list) {
        total += s.length() + 1;
    }

    total += 1;

    unique_ptr<wchar_t[]> rv = make_unique<wchar_t[]>(total);

    wchar_t* p = rv.get();
    for (const wstring& s : env_list) {
        size_t size = s.length();
        s.copy(p, size);
        p += size;
        *p = L'\0';
        p++;
    }
    *p = L'\0';
    return rv;
}

unique_ptr<wchar_t[]> build_env_block()
{
    list<wstring> env_list{};
    list<wstring> replace_list{};

    CIniConfig::GetEnvList(replace_list);
    parse_env(env_list);
    replace_env(env_list, replace_list);
    prefix_env(env_list, replace_list);
    unique_ptr<wchar_t[]> rv = env_list_to_block(env_list);
    return rv;
}