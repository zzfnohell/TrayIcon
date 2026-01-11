#pragma once
#include <map>
#include <memory>
#include <string>

std::string wstring_to_ansi(const std::wstring& wstr) noexcept;
std::wstring ansi_to_wstring(const std::string& str) noexcept;
std::wstring utf8_to_wstring(const std::string& str) noexcept;
std::string wstring_to_utf8(const std::wstring& wstr) noexcept;
std::unique_ptr<wchar_t[]> build_env_block(const std::map<std::wstring, std::wstring>& replacement) noexcept;
