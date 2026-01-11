#pragma once

extern const wchar_t kClassName[];

void register_class(HINSTANCE hInstance, WNDPROC WndProc) noexcept;

void unregister_class(HINSTANCE hInstance) noexcept;

HWND create_window(HINSTANCE hInstance, int nCmdShow) noexcept;
