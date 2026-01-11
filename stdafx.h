// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_IE // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600
#endif

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <Shellapi.h>
#include <Shlwapi.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <strsafe.h>
#include <tchar.h>
#include <windows.h>

// C RunTime Header Files
#include <algorithm>
#include <assert.h>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <list>
#include <memory.h>
#include <memory>
#include <optional>
#include <ranges>
#include <regex>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <string_view>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
