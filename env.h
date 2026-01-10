#pragma once
#include <memory>
#include <string>
#include <list>

std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& wstr);
std::unique_ptr<wchar_t[]> build_env_block(const std::list<std::wstring>& replace_list);