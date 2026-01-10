#include "stdafx.h"
#include "cfg.h"
using namespace std;

constexpr WCHAR ENV_DELIMITER = L'=';
constexpr WCHAR ENV_DELIMITER_S[] = L"=";

std::wstring utf8_to_wstring(const std::string& str)
{
    if (str.empty()) return L"";

    int size_needed = MultiByteToWideChar(
        CP_UTF8, 0,
        str.c_str(), (int)str.size(),
        nullptr, 0
    );

    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(
        CP_UTF8, 0,
        str.c_str(), (int)str.size(),
        wstr.data(), size_needed
    );

    return wstr;
}

std::string wstring_to_utf8(const std::wstring& wstr)
{
    if (wstr.empty()) return {};

    int size_needed = WideCharToMultiByte(
        CP_UTF8,                // convert to UTF-8
        0,                      // flags
        wstr.c_str(),           // source
        (int)wstr.size(),       // number of wide chars
        nullptr, 0,             // no output yet
        nullptr, nullptr
    );

    std::string str(size_needed, 0);

    WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.c_str(),
        (int)wstr.size(),
        str.data(),
        size_needed,
        nullptr, nullptr
    );

    return str;
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

void merge_env(list<wstring>& env_list, const list<wstring>& config_list) {

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
    const LPWCH  env = GetEnvironmentStrings();
    LPWCH p = env;
    while (*p) {
        const size_t n = wcslen(p);
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
        const size_t size = s.length();
        s.copy(p, size);
        p += size;
        *p = L'\0';
        p++;
    }
    *p = L'\0';
    return rv;
}

unique_ptr<wchar_t[]> build_env_block(const list<wstring> &replace_list)
{
    list<wstring> env_list{};
    parse_env(env_list);
    merge_env(env_list, replace_list);
    unique_ptr<wchar_t[]> rv = env_list_to_block(env_list);
    return rv;
}