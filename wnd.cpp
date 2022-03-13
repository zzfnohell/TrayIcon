#include "stdafx.h"
#include "wnd.h"

const wchar_t kClassName[] = L"TrayIconWindowClass";

void register_class(HINSTANCE hInstance, WNDPROC WndProc) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(CTLCOLOR_DLG);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = kClassName;
    wcex.hIconSm = NULL;
    BOOL rc = RegisterClassEx(&wcex);
    assert(rc);
}

void unregister_class(HINSTANCE hInstance) {
    BOOL rc = UnregisterClass(kClassName, hInstance);
    assert(rc);
}


HWND create_window(HINSTANCE hInstance, int nCmdShow)
{
    constexpr int kWidth = 400;
    constexpr int kHeight = 200;
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);
    int x = 0;
    int y = 0;

    HWND wnd = CreateWindow(
        kClassName,
        L"TrayIcon",
        WS_OVERLAPPEDWINDOW,
        x + (w - 400) / 2,
        y + (h - 200) / 2,
        400,
        200,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (wnd)
    {
        ShowWindow(wnd, nCmdShow);
        UpdateWindow(wnd);
    }

    return wnd;
}