#pragma once
#include <memory>
#include <string>
#include <list>

std::unique_ptr<wchar_t[]> build_env_block(const std::list<std::wstring>& replace_list);