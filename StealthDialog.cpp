// Win32 Dialog.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <assert.h>

#define TRAYICONID  1//             ID number for the Notify Icon
#define SWM_TRAYMSG WM_APP//        the message ID sent to our window

#define SWM_SHOW    WM_APP + 1//    show the window
#define SWM_HIDE    WM_APP + 2//    hide the window
#define SWM_EXIT    WM_APP + 3//    close the window


#define UWM_UPDATEINFO    (WM_USER + 4)
#define UWM_CHILDQUIT    (WM_USER + 5)

// Global Variables:
HINSTANCE       hInst;  // current instance
NOTIFYICONDATA  niData; // notify icon data
HWND hDlg;
// Forward declarations of functions included in this code module:
BOOL                InitInstance(HINSTANCE, int);
BOOL                OnInitDialog(HWND hWnd);
void                ShowContextMenu(HWND hWnd);
ULONGLONG           GetDllVersion(LPCTSTR lpszDllName);

STARTUPINFO si;
PROCESS_INFORMATION pi;
CString infoString;
CString cmdLine;

HANDLE hWaitProcessStartEvent = NULL;

HANDLE hProcessStartThread = NULL;

void CleanupThreadHandle()
{
    if (hProcessStartThread != NULL)
    {
        DWORD id = WaitForSingleObject(hProcessStartThread, INFINITE);
        assert(id == WAIT_OBJECT_0);
        CloseHandle(hProcessStartThread);
        hProcessStartThread = NULL;
    }
}

DWORD WINAPI ProcessHostThreadFunc(LPVOID lpParam)
{
    using namespace ATL;
    using namespace ATLPath;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    LPWSTR cmd = cmdLine.GetBuffer();
    BOOL succeeded;

    if (cmdLine.IsEmpty())
    {
        infoString.Format(L"CreateProcess failed (%d).\n", GetLastError());
        succeeded = PostMessage(hDlg, UWM_UPDATEINFO, NULL, NULL);
        assert(succeeded);
        return 1;
    }

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // Start the child process.
    if (!CreateProcess(
                NULL,   // No module name (use command line)
                cmd,        // Command line
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                TRUE,          // Set handle inheritance to FALSE
                0,              // No creation flags
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory
                &si,            // Pointer to STARTUPINFO structure
                &pi)           // Pointer to PROCESS_INFORMATION structure
       )
    {
        infoString.Format(L"CreateProcess failed (%d).\n", GetLastError());
        PostMessage(hDlg, UWM_UPDATEINFO, NULL, NULL);
        return 1;
    }

    infoString.Format(L"Process Running (%d).\n", pi.dwProcessId);
    succeeded = PostMessage(hDlg, UWM_UPDATEINFO, NULL, NULL);
    assert(succeeded);
    succeeded = SetEvent(hWaitProcessStartEvent);
    assert(succeeded);
    DWORD id = WaitForSingleObject(pi.hProcess, INFINITE);
    assert(id == WAIT_OBJECT_0);
    infoString.Format(L"Process Exited (%d).\n", pi.dwProcessId);
    succeeded = PostMessage(hDlg, UWM_UPDATEINFO, NULL, NULL);
    assert(succeeded);
    succeeded = CloseHandle(pi.hProcess);
    assert(succeeded);
    succeeded = CloseHandle(pi.hThread);
    assert(succeeded);
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    PostMessage(hDlg, UWM_CHILDQUIT, NULL, NULL);
    assert(succeeded);
    return 0;
}

void StartProcess()
{
    if (hProcessStartThread == NULL)
    {
        HANDLE handles[2] = { 0, 0 };
        DWORD id;
        hProcessStartThread = CreateThread(
                                  NULL,
                                  0,
                                  ProcessHostThreadFunc,
                                  NULL,
                                  0,
                                  &id);
        assert(hProcessStartThread != NULL);
        handles[0] = hProcessStartThread;
        handles[1] = hWaitProcessStartEvent;
        id = WaitForMultipleObjects(
                 sizeof(handles) / sizeof(HANDLE),
                 handles,
                 FALSE,
                 INFINITE);

        switch (id)
        {
            case WAIT_OBJECT_0:
                CleanupThreadHandle();
                break;

            case WAIT_OBJECT_0 + 1:
                break;

            default:
                assert(false);
        }
    }
}
void StopProcess()
{
    if (hProcessStartThread != NULL)
    {
        BOOL succeeded = TerminateProcess(pi.hProcess, 0);
        assert(succeeded);
        CleanupThreadHandle();
    }
}

INT_PTR CALLBACK    DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    MSG msg;
    HACCEL hAccelTable;
    LPCWSTR cmd = GetCommandLine();
    int argc;
    LPWSTR *argv = CommandLineToArgvW(cmd, &argc);
    cmdLine.Empty();

    for (int i = 1; i < argc; i++)
    {
        cmdLine.Append(argv[i]);
        cmdLine.AppendChar(L' ');
    }

    hWaitProcessStartEvent = CreateEvent(
                                 NULL,
                                 FALSE,
                                 FALSE,
                                 NULL);
    assert(hWaitProcessStartEvent);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_STEALTHDIALOG);

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) &&
                !IsDialogMessage(msg.hwnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CloseHandle(hWaitProcessStartEvent);
    CloseHandle(hProcessStartThread);
    return (int) msg.wParam;
}

//  Initialize the window and tray icon
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    WCHAR name[MAX_PATH];
    BOOL succeeded;
    succeeded = GetModuleFileName(NULL, name, MAX_PATH);
    assert(succeeded);
    CPath path = name;
    // prepare for XP style controls
    InitCommonControls();
    // store instance handle and create dialog
    hInst = hInstance;
    hDlg = CreateDialog(
               hInstance,
               MAKEINTRESOURCE(IDD_DLG_DIALOG),
               NULL,
               (DLGPROC)DlgProc);

    if (!hDlg)
    {
        return FALSE;
    }

    // Fill the NOTIFYICONDATA structure and call Shell_NotifyIcon
    // zero the structure - note:   Some Windows funtions require this but
    //                              I can't be bothered which ones do and
    //                              which ones don't.
    ZeroMemory(&niData, sizeof(NOTIFYICONDATA));
    ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));

    if (ullVersion >= MAKEDLLVERULL(5, 0, 0, 0))
    {
        niData.cbSize = sizeof(NOTIFYICONDATA);
    }
    else
    {
        niData.cbSize = NOTIFYICONDATA_V2_SIZE;
    }

    CPath imagePath = path;
    imagePath.RemoveFileSpec();
    imagePath.Append(L"image.ico");
    niData.uID = TRAYICONID;
    niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    niData.hIcon = (HICON)LoadImage(
                       NULL,
                       imagePath,
                       IMAGE_ICON,
                       GetSystemMetrics(SM_CXSMICON),
                       GetSystemMetrics(SM_CYSMICON),
                       LR_DEFAULTCOLOR | LR_LOADFROMFILE);
    // the window to send messages to and the message to send
    //      note:   the message value should be in the
    //              range of WM_APP through 0xBFFF
    niData.hWnd = hDlg;
    niData.uCallbackMessage = SWM_TRAYMSG;
    // tooltip message
    lstrcpyn(niData.szTip,
             _T("Time flies like an arrow but\n   fruit flies like a banana!"),
             sizeof(niData.szTip) / sizeof(TCHAR));
    Shell_NotifyIcon(NIM_ADD, &niData);

    // free icon handle
    if (niData.hIcon && DestroyIcon(niData.hIcon))
    {
        niData.hIcon = NULL;
    }

    // call ShowWindow here to make the dialog initially visible
    return TRUE;
}

BOOL OnInitDialog(HWND hWnd)
{
    WCHAR name[MAX_PATH];
    BOOL succeeded;
    succeeded = GetModuleFileName(NULL, name, MAX_PATH);
    assert(succeeded);
    CPath path = name;
    CPath imagePath = path;
    imagePath.RemoveFileSpec();
    imagePath.Append(L"image.ico");
    HMENU hMenu = GetSystemMenu(hWnd, FALSE);

    if (hMenu)
    {
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T("About"));
    }

    HICON hIcon = (HICON)LoadImage(
                      NULL,
                      imagePath,
                      IMAGE_ICON,
                      GetSystemMetrics(SM_CXSMICON),
                      GetSystemMetrics(SM_CYSMICON),
                      LR_DEFAULTCOLOR | LR_LOADFROMFILE);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    StartProcess();
    return TRUE;
}

// Name says it all
void ShowContextMenu(HWND hWnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();

    if (hMenu)
    {
        if (IsWindowVisible(hWnd))
        {
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_HIDE, _T("Hide"));
        }
        else
        {
            InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, _T("Show"));
        }

        InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, _T("Exit"));
        // note:    must set window to the foreground or the
        //          menu won't disappear when it should
        SetForegroundWindow(hWnd);
        TrackPopupMenu(
            hMenu,
            TPM_BOTTOMALIGN,
            pt.x,
            pt.y,
            0,
            hWnd,
            NULL);
        DestroyMenu(hMenu);
    }
}

// Get dll version number
ULONGLONG GetDllVersion(LPCTSTR lpszDllName)
{
    ULONGLONG ullVersion = 0;
    HINSTANCE hinstDll;
    hinstDll = LoadLibrary(lpszDllName);

    if (hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");

        if (pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;
            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            hr = (*pDllGetVersion)(&dvi);

            if (SUCCEEDED(hr))
            {
                ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion, 0, 0);
            }
        }

        FreeLibrary(hinstDll);
    }

    return ullVersion;
}

void UpdateInfoText()
{
    HWND wnd = GetDlgItem(hDlg, IDC_STATIC_INFO);
    LPCWSTR str = infoString;
    SetWindowText(wnd, str);
}

// Message handler for the app
INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;

    switch (message)
    {
        case SWM_TRAYMSG:
            switch (lParam)
            {
                case WM_LBUTTONDBLCLK:
                    UpdateInfoText();
                    ShowWindow(hWnd, SW_RESTORE);
                    break;

                case WM_RBUTTONDOWN:
                case WM_CONTEXTMENU:
                    ShowContextMenu(hWnd);
            }

            break;

        case UWM_UPDATEINFO:
            UpdateInfoText();
            break;

        case UWM_CHILDQUIT:
            CleanupThreadHandle();
            break;

        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) == SC_MINIMIZE)
            {
                ShowWindow(hWnd, SW_HIDE);
                return 1;
            }
            else if (wParam == IDM_ABOUT)
            {
                DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
            }

            break;

        case WM_COMMAND:
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);

            switch (wmId)
            {
                case SWM_SHOW:
                    ShowWindow(hWnd, SW_RESTORE);
                    break;

                case SWM_HIDE:
                case IDOK:
                    ShowWindow(hWnd, SW_HIDE);
                    break;

                case IDSTART:
                    StartProcess();
                    break;

                case IDSTOP:
                    StopProcess();
                    break;

                case SWM_EXIT:
                    DestroyWindow(hWnd);
                    break;

                case IDM_ABOUT:
                    DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
                    break;
            }

            return 1;

        case WM_INITDIALOG:
            return OnInitDialog(hWnd);

        case WM_CLOSE:
            ShowWindow(hWnd, SW_HIDE);
            break;

        case WM_DESTROY:
            niData.uFlags = 0;
            Shell_NotifyIcon(NIM_DELETE, &niData);
            PostQuitMessage(0);
            break;
    }

    return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return TRUE;
            }

            break;
    }

    return FALSE;
}
