// Win32 Dialog.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <assert.h>
#include <cstdio>

#include "iniconfig.h"
#include "env.h"

using namespace std;

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

LPWSTR str_alloc(size_t size) {
    size_t n = (size + 1) * sizeof(WCHAR);
    LPWSTR p = (LPWSTR)malloc(n);
    return p;
}

LPCWSTR str_format(LPCWSTR format, ...) {
    constexpr int K = 4;
    LPWSTR p = NULL;
    va_list args;
    va_start(args, format);
    wchar_t b[K];
    size_t n = vswprintf_s(b, format, args);
    p = str_alloc(n);
    size_t buf_size = (n + 1) * sizeof(WCHAR);
    if (n >= K) {
        n = vswprintf(p, buf_size, format, args);
    }
    else {
        wcscpy_s(p, n + 1, b);
    }
    va_end(args);

    return NULL;
}

LPWSTR format_last_error(const wstring& msg) {
    wostringstream oss;
    oss << msg << " " << L"(" << GetLastError() << ")" << endl;
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

    WCHAR* app_args = new WCHAR[APP_ARGS_LENGTH];
    CIniConfig::GetAppArgs(app_args, APP_ARGS_LENGTH);
    delete[] app_args;
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
        0, // No creation flags
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
    config.Initialize(); 

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

    return (int)msg.wParam;
}

//  Initialize the window and tray icon
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    // prepare for XP style controls
    InitCommonControls();
    // store instance handle and create dialog
    kInst = hInstance;
    kDlg = CreateDialog(
        hInstance,
        MAKEINTRESOURCE(IDD_DLG_DIALOG),
        NULL,
        (DLGPROC)DlgProc);

    if (!kDlg)
    {
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
        AppendMenu(h_menu, MF_STRING, IDM_ABOUT, _T("About"));
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

void UpdateInfoText(LPCWSTR s)
{
    HWND wnd = GetDlgItem(kDlg, IDC_STATIC_INFO);
    SetWindowText(wnd, s);
    delete[] s;
}

void UpdateInfoTextByConst(LPCWSTR s)
{
    HWND wnd = GetDlgItem(kDlg, IDC_STATIC_INFO);
    SetWindowText(wnd, s);
}

// Message handler for the app
INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        else if (wParam == IDM_ABOUT)
        {
            DialogBox(kInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
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

        case IDM_ABOUT:
            DialogBox(kInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
            break;
        default:
            break;
        }

        return 1;

    case WM_INITDIALOG:
        return OnInitDialog(hWnd);

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
    default:
        break;
    }

    return FALSE;
}
