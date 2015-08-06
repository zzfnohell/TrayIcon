// Win32 Dialog.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <assert.h>
#include "Config.h"

#define SWM_TRAYMSG WM_APP//        the message ID sent to our window

#define SWM_SHOW    WM_APP + 1//    show the window
#define SWM_HIDE    WM_APP + 2//    hide the window
#define SWM_EXIT    WM_APP + 3//    close the window


#define UWM_UPDATEINFO    (WM_USER + 4)
#define UWM_CHILDQUIT    (WM_USER + 5)


using namespace ATL;

CConfig config;

// Global Variables:
HINSTANCE       hInst;  // current instance


NOTIFYICONDATA  niData ;

HWND hDlg;
// Forward declarations of functions included in this code module:
BOOL                InitInstance(HINSTANCE, int);
BOOL                OnInitDialog(HWND hWnd);
void                ShowContextMenu(HWND hWnd);

CString infoString;
CString cmdLine;
HANDLE job;
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

void IntializeNotificationData()
{
    ZeroMemory(&niData, sizeof(NOTIFYICONDATA));
    niData.cbSize = sizeof(NOTIFYICONDATA);
    niData.uID = 100;
    niData.uFlags = NIF_ICON | NIF_MESSAGE;
    niData.hWnd = hDlg;
    niData.uCallbackMessage = SWM_TRAYMSG;
    niData.uVersion = NOTIFYICON_VERSION_4;
}

void DestroyNotificationData()
{
}

DWORD WINAPI ProcessHostThreadFunc(LPVOID lpParam)
{
    using namespace ATL;
    using namespace ATLPath;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    LPWSTR cmd = cmdLine.GetBuffer();
    BOOL succeeded;
    job = CreateJobObject(NULL, NULL);
    assert(job != NULL);

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

    succeeded = AssignProcessToJobObject(job, pi.hProcess);
    assert(succeeded);
    infoString.Format(L"Process Running (%d).\n", pi.dwProcessId);
    succeeded = PostMessage(hDlg, UWM_UPDATEINFO, NULL, NULL);
    assert(succeeded);
    succeeded = SetEvent(hWaitProcessStartEvent);
    assert(succeeded);
    DWORD id = WaitForSingleObject(job, INFINITE);
    assert(id == WAIT_OBJECT_0);
    infoString.Format(L"Process Exited (%d).\n", pi.dwProcessId);
    succeeded = PostMessage(hDlg, UWM_UPDATEINFO, NULL, NULL);
    assert(succeeded);
    succeeded = CloseHandle(pi.hProcess);
    assert(succeeded);
    succeeded = CloseHandle(pi.hThread);
    assert(succeeded);
    succeeded = CloseHandle(job);
    assert(succeeded);
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    PostMessage(hDlg, UWM_CHILDQUIT, NULL, NULL);
    assert(succeeded);
    return 0;
}

void ShowNotificationData(bool on)
{
    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(nid));
    const CPath * imagePath = on ? config.GetOnIconPath() : config.GetOffIconPath();
    UINT flags = LR_MONOCHROME;
    flags |= LR_LOADFROMFILE;
    HICON icon = (HICON)LoadImage(
                     NULL,
                     *imagePath,
                     IMAGE_ICON,
                     GetSystemMetrics(SM_CXSMICON),
                     GetSystemMetrics(SM_CYSMICON),
                     flags);
    nid.hIcon = icon;
    nid.uID = niData.uID;
    nid.hWnd = niData.hWnd;
    nid.uFlags = NIF_ICON;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
    DestroyIcon(icon);
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

    ShowNotificationData(true);
}

void StopProcess()
{
    if (hProcessStartThread != NULL)
    {
        BOOL succeeded = TerminateJobObject(job, -1);
        assert(succeeded);
        CleanupThreadHandle();
    }

    ShowNotificationData(false);
}

INT_PTR CALLBACK    DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void BuildCmdLine()
{
    const CPath *appPath = config.GetAppPath();
    const CString *args = config.GetAppArgs();
    cmdLine = static_cast<CString>(*appPath);
    cmdLine.AppendChar(L' ');
    cmdLine.Append(*args);
}

int APIENTRY _tWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    MSG msg;
    HACCEL hAccelTable;
    config.Initialize();
    BuildCmdLine();
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
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg) ||
                !IsDialogMessage(msg.hwnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CloseHandle(hWaitProcessStartEvent);
    CloseHandle(hProcessStartThread);
    DestroyNotificationData();
    return (int) msg.wParam;
}



//  Initialize the window and tray icon
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
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

    IntializeNotificationData();
    const CPath * imagePath =  config.GetOffIconPath();
    UINT flags = LR_LOADFROMFILE;
    HICON icon = (HICON)LoadImage(
                     NULL,
                     *imagePath,
                     IMAGE_ICON,
                     GetSystemMetrics(SM_CXSMICON),
                     GetSystemMetrics(SM_CYSMICON),
                     flags);
    niData.hIcon = icon;
    Shell_NotifyIcon(NIM_ADD, &niData);
    DestroyIcon(icon);
    niData.hIcon = NULL;
    StartProcess();
    return TRUE;
}

BOOL OnInitDialog(HWND hWnd)
{
    const CPath *imagePath = config.GetOffIconPath();
    HMENU hMenu = GetSystemMenu(hWnd, FALSE);

    if (hMenu)
    {
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T("About"));
    }

    HICON hIcon = (HICON)LoadImage(
                      NULL,
                      *imagePath,
                      IMAGE_ICON,
                      GetSystemMetrics(SM_CXSMICON),
                      GetSystemMetrics(SM_CYSMICON),
                      LR_LOADFROMFILE);
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    DestroyIcon(hIcon);
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
            ShowNotificationData(false);
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
                    ShowWindow(hWnd, SW_HIDE);
                    break;

                case IDC_START:
                    StartProcess();
                    break;

                case IDC_STOP:
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
            StopProcess();
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
