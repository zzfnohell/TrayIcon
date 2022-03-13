// Win32 Dialog.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <assert.h>
#include <cstdio>

#include "iniconfig.h"
#include "env.h"
#include "wnd.h"
using namespace std;

#define IDC_START 2001
#define IDC_STOP  2002

#define SWM_TRAYMSG WM_APP//        the message ID sent to our window

#define SWM_SHOW    (WM_APP + 1)//    show the window
#define SWM_HIDE    (WM_APP + 2)//    hide the window
#define SWM_EXIT    (WM_APP + 3)//    close the window

#define UWM_UPDATEINFO    (WM_USER + 4)
#define UWM_CHILDQUIT    (WM_USER + 5)
#define UWM_CHILDCREATE    (WM_USER + 6)

CIniConfig config{};

// Global Variables:
HINSTANCE kInst; // current instance

NOTIFYICONDATA kData;

HWND kDlg;
// Forward declarations of functions included in this code module:
static BOOL InitInstance(HINSTANCE, int);
static BOOL OnInitDialog(HWND hWnd);
static void ShowContextMenu(HWND hWnd);

HANDLE hJob;
HANDLE hNewWaitHandle;
bool createJob = false;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


LPWSTR format_last_error(const wstring& msg) {
    wostringstream oss;
    DWORD dwErrorCode = GetLastError();
    WCHAR pBuffer[MAX_PATH];
    DWORD cchMsg = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,  /* (not used with FORMAT_MESSAGE_FROM_SYSTEM) */
        dwErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        pBuffer,
        MAX_PATH,
        NULL);

    oss << msg << endl << L"(" << dwErrorCode << ")" << " " << pBuffer;
    wstring s = oss.str();
    rsize_t word_count = s.length() + 1;
    wchar_t* p = new wchar_t[word_count];
    wcscpy_s(p, word_count, s.c_str());
    return p;
}

LPWSTR format_process_id(const wstring& msg, DWORD process_id) {
    wostringstream oss;
    oss << msg << " " << L"(" << GetLastError() << ")" << endl;
    wstring s = oss.str();
    rsize_t word_count = s.length() + 1;
    wchar_t* p = new wchar_t[word_count];
    wcscpy_s(p, word_count, s.c_str());
    return p;
}


VOID CALLBACK WaitOrTimerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired)
{
    if (!TimerOrWaitFired)
    {
        [[maybe_unused]] const BOOL succeeded = PostMessage(kDlg, UWM_CHILDQUIT, NULL, NULL);
        assert(succeeded);
    }

    return;
}

void InitializeNotificationData()
{
    ZeroMemory(&kData, sizeof(NOTIFYICONDATA));
    kData.cbSize = sizeof(NOTIFYICONDATA);
    kData.uID = 100;
    kData.uFlags = NIF_ICON | NIF_MESSAGE;
    kData.hWnd = kDlg;
    kData.uCallbackMessage = SWM_TRAYMSG;
    kData.uVersion = NOTIFYICON_VERSION_4;
}

void ShowNotificationData(bool on)
{
    NOTIFYICONDATA nid;
    WCHAR image_path[MAX_PATH];

    ZeroMemory(&nid, sizeof(nid));
    on ? CIniConfig::GetOnIconPath(image_path) : CIniConfig::GetOffIconPath(image_path);

    UINT flags = LR_MONOCHROME;
    flags |= LR_LOADFROMFILE;
    const auto icon = (HICON)LoadImage(
        NULL,
        image_path,
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        flags);
    nid.hIcon = icon;
    nid.uID = kData.uID;
    nid.hWnd = kData.hWnd;
    nid.uFlags = NIF_ICON;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
    DestroyIcon(icon);
}

constexpr int APP_ARGS_LENGTH = 2048;

unique_ptr<wchar_t[]> BuildCmdLine()
{
    WCHAR app_path[MAX_PATH];
    CIniConfig::GetAppPath(app_path);

    unique_ptr<wchar_t[]> app_args = make_unique<wchar_t[]>(APP_ARGS_LENGTH);
    CIniConfig::GetAppArgs(app_args.get(), APP_ARGS_LENGTH);

    wstringstream wss;
    wss << app_path << " " << app_args;

    wstring s = wss.str();

    size_t word_count = s.length() + 1;
    unique_ptr<wchar_t[]> cmd_line = make_unique<wchar_t[]>(word_count);
    s.copy(cmd_line.get(), word_count - 1);
    s[word_count - 1] = L'\0';
    return cmd_line;
}

void StartProcess()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    LPWSTR msg;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    BOOL rc;

    HANDLE hJobObject = CreateJobObject(NULL, NULL);
    assert(hJobObject != NULL);

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli{};
    memset(&jeli, 0, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));

    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    rc = SetInformationJobObject(hJobObject, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
    if (rc == 0)
    {
        // ReSharper disable once CppAssignedValueIsNeverUsed
        rc = CloseHandle(hJobObject);
        assert(rc);

        msg = format_last_error(L"Create Job failed.");
        // ReSharper disable once CppAssignedValueIsNeverUsed
        rc = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(msg));
        assert(rc);
        return;
    }

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    WCHAR startup_dir[MAX_PATH];
    CIniConfig::GetWorkDirPath(startup_dir);

    unique_ptr<wchar_t[]>  cmd_line = BuildCmdLine();

    unique_ptr<wchar_t[]> env_block = build_env_block();
    rc = CreateProcess(
        NULL, // No module name (use command line)
        cmd_line.get(), // Command line
        NULL, // Process handle not inheritable
        NULL, // Thread handle not inheritable
        TRUE, // Set handle inheritance to FALSE
        CREATE_UNICODE_ENVIRONMENT, // No creation flags
        env_block.get(), // Use parent's environment block
        startup_dir, // Use parent's starting directory
        &si, // Pointer to STARTUPINFO structure
        &pi // Pointer to PROCESS_INFORMATION structure
    );

    if (!rc)
    {
        // ReSharper disable once CppAssignedValueIsNeverUsed
        rc = CloseHandle(hJobObject);
        assert(rc);

        msg = format_last_error(L"CreateProcess failed");
        // ReSharper disable once CppAssignedValueIsNeverUsed
        rc = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(msg));
        assert(rc);
        return;
    }

    rc = AssignProcessToJobObject(hJobObject, pi.hProcess);
    if (!rc)
    {
        msg = format_last_error(L"AssignProcessToJobObject failed.");
        // ReSharper disable once CppAssignedValueIsNeverUsed
        rc = CloseHandle(pi.hProcess);
        assert(rc);
        // ReSharper disable once CppAssignedValueIsNeverUsed
        rc = CloseHandle(hJobObject);
        assert(rc);
        // ReSharper disable once CppAssignedValueIsNeverUsed
        rc = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(msg));
        assert(rc);
        return;
    }

    // ReSharper disable once CppAssignedValueIsNeverUsed
    rc = PostMessage(kDlg, UWM_CHILDCREATE, NULL, reinterpret_cast<LPARAM>(hJobObject));
    assert(rc);

    msg = format_process_id(L"Process Running", pi.dwProcessId);
    // ReSharper disable once CppAssignedValueIsNeverUsed
    rc = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(msg));
    assert(rc);

    // ReSharper disable once CppAssignedValueIsNeverUsed
    rc = RegisterWaitForSingleObject(&hNewWaitHandle, pi.hProcess, WaitOrTimerCallback, hJobObject, INFINITE,
        WT_EXECUTEONLYONCE);
    assert(rc);
}

void StopProcess()
{
    if (hJob)
    {
        // ReSharper disable once CppEntityAssignedButNoRead
        // ReSharper disable once CppJoinDeclarationAndAssignment
        BOOL succeeded;

        // ReSharper disable once CppAssignedValueIsNeverUsed
        succeeded = TerminateJobObject(hJob, -1);
        assert(succeeded);

        // ReSharper disable once CppAssignedValueIsNeverUsed
        succeeded = CloseHandle(hJob);
        assert(succeeded);
    }
}

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    MSG msg;
    HACCEL hAccelTable;
    register_class(hInstance, WndProc);
    config.Initialize();

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    // Main message loop:
    BOOL fGotMessage;
    while ((fGotMessage = GetMessage(&msg, (HWND)NULL, 0, 0)) != 0 && fGotMessage != -1)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    unregister_class(hInstance);
    return (int)msg.wParam;
}

//  Initialize the window and tray icon
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    // prepare for XP style controls
    //InitCommonControls();

    kInst = hInstance;

    kDlg = create_window(hInstance, nCmdShow);
    assert(kDlg);
    if (!kDlg)
    {
        LPWSTR msg = format_last_error(L"Create window failed.");
        MessageBox(NULL, msg, L"ERROR", MB_ICONERROR | MB_OK);
        delete[] msg;
        return FALSE;
    }

    InitializeNotificationData();

    WCHAR image_path[MAX_PATH];
    CIniConfig::GetOffIconPath(image_path);

    constexpr UINT flags = LR_LOADFROMFILE;
    const auto icon = (HICON)LoadImage(
        NULL,
        image_path,
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        flags);
    kData.hIcon = icon;
    Shell_NotifyIcon(NIM_ADD, &kData);
    DestroyIcon(icon);
    kData.hIcon = NULL;
    StartProcess();
    return TRUE;
}

BOOL OnInitDialog(HWND hWnd)
{
    WCHAR image_path[MAX_PATH];
    CIniConfig::GetOffIconPath(image_path);

    if (HMENU h_menu = GetSystemMenu(hWnd, FALSE))
    {
        AppendMenu(h_menu, MF_SEPARATOR, 0, NULL);
    }

    auto hIcon = (HICON)LoadImage(
        NULL,
        image_path,
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

    if (HMENU h_menu = CreatePopupMenu())
    {
        if (IsWindowVisible(hWnd))
        {
            InsertMenu(h_menu, -1, MF_BYPOSITION, SWM_HIDE, _T("Hide"));
        }
        else
        {
            InsertMenu(h_menu, -1, MF_BYPOSITION, SWM_SHOW, _T("Show"));
        }

        InsertMenu(h_menu, -1, MF_BYPOSITION, SWM_EXIT, _T("Exit"));
        // note:    must set window to the foreground or the
        //          menu won't disappear when it should
        SetForegroundWindow(hWnd);
        TrackPopupMenu(
            h_menu,
            TPM_BOTTOMALIGN,
            pt.x,
            pt.y,
            0,
            hWnd,
            NULL);
        DestroyMenu(h_menu);
    }
}

HWND wMsg = NULL;
void UpdateInfoText(LPCWSTR s)
{
    SetWindowText(wMsg, s);
    delete[] s;
}

void UpdateInfoTextByConst(LPCWSTR s)
{
    SetWindowText(wMsg, s);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wm_id;

    switch (message)
    {
    case SWM_TRAYMSG:
        switch (lParam)
        {
        case WM_LBUTTONDBLCLK:
            ShowWindow(hWnd, SW_RESTORE);
            break;

        case WM_RBUTTONDOWN:
        case WM_CONTEXTMENU:
            ShowContextMenu(hWnd);
            break;
        default:
            break;
        }

        break;

    case UWM_UPDATEINFO:
        UpdateInfoText(reinterpret_cast<LPCWSTR>(lParam));
        break;

    case UWM_CHILDQUIT:
        if (hJob)
        {
            CloseHandle(hJob);
            hJob = NULL;
        }
        ShowNotificationData(false);

        UpdateInfoTextByConst(L"Process Exit.");
        break;
    case UWM_CHILDCREATE:
        hJob = reinterpret_cast<HANDLE>(lParam);  // NOLINT(performance-no-int-to-ptr)
        ShowNotificationData(true);
        break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xFFF0) == SC_MINIMIZE)
        {
            ShowWindow(hWnd, SW_HIDE);
            return 1;
        }

        break;

    case WM_COMMAND:
        wm_id = LOWORD(wParam);

        switch (wm_id)
        {
        case SWM_SHOW:
            ShowWindow(hWnd, SW_RESTORE);
            break;

        case SWM_HIDE:
            ShowWindow(hWnd, SW_HIDE);
            break;

        case IDC_START:
            if (!hJob)
            {
                StartProcess();
            }
            break;

        case IDC_STOP:
            StopProcess();
            hJob = NULL;
            break;

        case SWM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            break;
        }

        return 1;

    case WM_CREATE:
        wMsg = CreateWindow(
            L"STATIC",
            L"Ready to Start",
            WS_VISIBLE | WS_CHILD,
            50, 20, 300, 60,
            hWnd, NULL, 0, NULL);
        CreateWindow(
            L"BUTTON", L"开始", WS_VISIBLE | WS_CHILD,
            80, 120, 100, 20, hWnd, (HMENU)IDC_START, 0, NULL);
        CreateWindow(
            L"BUTTON", L"停止", WS_VISIBLE | WS_CHILD,
            200, 120, 100, 20, hWnd, (HMENU)IDC_STOP, 0, NULL);
        OnInitDialog(hWnd);
        break;

    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        break;

    case WM_DESTROY:
        ShowNotificationData(false);
        kData.uFlags = 0;
        Shell_NotifyIcon(NIM_DELETE, &kData);
        StopProcess();
        hJob = NULL;
        PostQuitMessage(0);
        break;
    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}