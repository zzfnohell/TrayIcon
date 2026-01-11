#include "stdafx.h"
#include "cfg.h"
#include <map>

using namespace std;

using EnvEntry = std::pair<std::wstring, std::wstring>;
using EnvMap = std::map<std::wstring, EnvEntry>;

std::wstring to_lower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) { return std::tolower(c); });
    return s;
}

std::string wstring_to_ansi(const std::wstring& wstr) {
    if (wstr.empty())
        return "";

    // Step 1: get required size
    int size_needed = WideCharToMultiByte(CP_ACP,               // system ANSI code page
                                          WC_NO_BEST_FIT_CHARS, // avoid best-fit mapping
                                          wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0,
                                          nullptr, // default char if unmappable
                                          nullptr  // receives "used default char" flag
    );

    if (size_needed <= 0)
        return ""; // or throw

    // Step 2: convert
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wstr.c_str(), static_cast<int>(wstr.size()), str.data(),
                        size_needed, nullptr, nullptr);

    return str;
}
std::wstring ansi_to_wstring(const std::string& str) {
    if (str.empty())
        return L"";

    // Step 1: get size needed
    int size_needed = MultiByteToWideChar(CP_ACP,               // system ANSI code page
                                          MB_ERR_INVALID_CHARS, // fail on invalid chars
                                          str.c_str(), static_cast<int>(str.size()), nullptr, 0);

    if (size_needed <= 0)
        return L""; // or throw

    // Step 2: convert
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, str.c_str(), static_cast<int>(str.size()), wstr.data(),
                        size_needed);

    return wstr;
}

std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty())
        return L"";

    int size_needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), (int)str.size(), nullptr, 0);

    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), (int)str.size(), wstr.data(), size_needed);

    return wstr;
}

std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty())
        return {};

    int size_needed = WideCharToMultiByte(CP_UTF8,              // convert to UTF-8
                                          MB_ERR_INVALID_CHARS, // flags
                                          wstr.c_str(),         // source
                                          (int)wstr.size(),     // number of wide chars
                                          nullptr, 0,           // no output yet
                                          nullptr, nullptr);

    std::string str(size_needed, 0);

    WideCharToMultiByte(CP_UTF8, MB_ERR_INVALID_CHARS, wstr.c_str(), (int)wstr.size(), str.data(), size_needed, nullptr,
                        nullptr);

    return str;
}
optional<tuple<wstring, wstring>> try_match(const wstring& s) {
    auto pos = s.find(L'=');
    if (pos == std::wstring::npos)
        return std::nullopt;

    return std::make_tuple(s.substr(0, pos), s.substr(pos + 1));
}

bool iequals(const std::wstring_view& a, const std::wstring_view& b) {
    if (a.size() != b.size())
        return false;
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      [](wchar_t a, wchar_t b) { return tolower(a) == tolower(b); });
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
    const LPWCH env = GetEnvironmentStrings();
    LPWCH p = env;
    while (*p) {
        const size_t n = wcslen(p);
        wstring s{p, n};
        env_list.emplace_back(s);
        p += n + 1;
    }

    BOOL rt = FreeEnvironmentStrings(env);
    assert(rt);
}

unique_ptr<wchar_t[]> env_list_to_block(const EnvMap& env) {
    size_t total = 1; // final double-null terminator

    for (const auto& kv : env) {
        const auto& name = kv.second.first;
        const auto& value = kv.second.second;

        total += name.size() + 1 + value.size() + 1;
    }

    auto block = std::make_unique<wchar_t[]>(total);
    wchar_t* p = block.get();

    for (const auto& kv : env) {
        const auto& name = kv.second.first;
        const auto& value = kv.second.second;

        std::wstring line = name + L"=" + value;

        std::copy(line.begin(), line.end(), p);
        p += line.size();
        *p++ = L'\0';
    }

    *p = L'\0';

    return block;
}

bool split_env_pair(const std::wstring& s, std::wstring& name, std::wstring& value) {
    size_t pos = s.find(L'=');
    if (pos == std::wstring::npos)
        return false;

    name = s.substr(0, pos);
    value = s.substr(pos + 1);
    return true;
}

EnvMap env_list_to_map(const std::list<std::wstring>& env_list) {
    EnvMap env;

    for (const auto& s : env_list) {
        std::wstring name, value;

        if (!split_env_pair(s, name, value))
            continue;

        env[to_lower(name)] = std::make_pair(name, value);
    }

    return env;
}

void merge_env_map(EnvMap& env, const EnvMap& cfg) {
    for (const auto& kv : cfg) {
        env[kv.first] = kv.second; // overwrite or insert
    }
}

EnvMap config_map_from_user_map(const std::map<std::wstring, std::wstring>& replacement) {
    EnvMap cfg;

    for (const auto& kv : replacement) {
        const auto& name = kv.first;
        const auto& value = kv.second;

        cfg[to_lower(name)] = std::make_pair(name, value);
    }

    return cfg;
}

unique_ptr<wchar_t[]> build_env_block(const map<wstring, wstring>& replacement) {
    list<wstring> env_list{};
    parse_env(env_list);
    EnvMap env = env_list_to_map(env_list);
    EnvMap cfg = config_map_from_user_map(replacement);
    merge_env_map(env, cfg);
    unique_ptr<wchar_t[]> rv = env_list_to_block(env);
    return rv;
}
