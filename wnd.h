#pragma once

extern const wchar_t kClassName[];

void register_class(HINSTANCE hInstance, WNDPROC WndProc);

void unregister_class(HINSTANCE hInstance);

HWND create_window(HINSTANCE hInstance, int nCmdShow);