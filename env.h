#pragma once
#include <map>
#include <memory>
#include <string>

std::string wstring_to_ansi(const std::wstring& wstr);
std::wstring ansi_to_wstring(const std::string& str);
std::wstring utf8_to_wstring(const std::string& str);
std::string wstring_to_utf8(const std::wstring& wstr);
std::unique_ptr<wchar_t[]> build_env_block(const std::map<std::wstring, std::wstring>& replacement);
