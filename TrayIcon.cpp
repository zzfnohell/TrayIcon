// Win32 Dialog.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include <assert.h>
#include "config.h"

#define SWM_TRAYMSG WM_APP//        the message ID sent to our window

#define SWM_SHOW    (WM_APP + 1)//    show the window
#define SWM_HIDE    (WM_APP + 2)//    hide the window
#define SWM_EXIT    (WM_APP + 3)//    close the window

#define UWM_UPDATEINFO    (WM_USER + 4)
#define UWM_CHILDQUIT    (WM_USER + 5)
#define UWM_CHILDCREATE    (WM_USER + 6)

CConfig config{};

// Global Variables:
HINSTANCE kInst; // current instance

NOTIFYICONDATA kData;

HWND kDlg;
// Forward declarations of functions included in this code module:
static BOOL InitInstance(HINSTANCE, int);
static BOOL OnInitDialog(HWND hWnd);
static void ShowContextMenu(HWND hWnd);

TCHAR kMsg[MAX_PATH] = {0};
TCHAR kCmdLine[MAX_PATH] = {0};

HANDLE hJob;
HANDLE hNewWaitHandle;
bool createJob = false;

VOID CALLBACK WaitOrTimerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired)
{
	if (!TimerOrWaitFired)
	{
		const BOOL succeeded = PostMessage(kDlg, UWM_CHILDQUIT, NULL, NULL);
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
	ZeroMemory(&nid, sizeof(nid));
	const LPCWSTR image_path = on ? config.GetOnIconPath() : config.GetOffIconPath();
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

void StartProcess()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	BOOL succeeded;

	if (!(*kCmdLine))
	{
		swprintf_s(kMsg, L"Command line is emtpy (%d).\n", GetLastError());
		succeeded = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(kMsg));
		assert(succeeded);
		return;
	}
	HANDLE hJobObject = CreateJobObject(NULL, NULL);
	assert(hJobObject != NULL);

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {0};
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	succeeded = SetInformationJobObject(hJobObject, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
	if (succeeded == 0)
	{
		succeeded = CloseHandle(hJobObject);
		assert(succeeded);
		swprintf_s(kMsg, L"Create Job failed (%d).\n", GetLastError());
		succeeded = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(kMsg));
		assert(succeeded);
		return;
	}

	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	const auto startup_dir = config.GetWorkDirPath();
	succeeded = CreateProcess(
		NULL, // No module name (use command line)
		kCmdLine, // Command line
		NULL, // Process handle not inheritable
		NULL, // Thread handle not inheritable
		TRUE, // Set handle inheritance to FALSE
		0, // No creation flags
		NULL, // Use parent's environment block
		startup_dir, // Use parent's starting directory
		&si, // Pointer to STARTUPINFO structure
		&pi // Pointer to PROCESS_INFORMATION structure
	);

	if (!succeeded)
	{
		succeeded = CloseHandle(hJobObject);
		assert(succeeded);
		wsprintf(kMsg, L"CreateProcess failed (%d).\n", GetLastError());
		succeeded = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(kMsg));
		assert(succeeded);
		return;
	}

	succeeded = AssignProcessToJobObject(hJobObject, pi.hProcess);
	if (!succeeded)
	{
		wsprintf(kMsg, L"AssignProcessToJobObject failed (%d).\n", GetLastError());
		succeeded = CloseHandle(pi.hProcess);
		assert(succeeded);
		succeeded = CloseHandle(hJobObject);
		assert(succeeded);
		succeeded = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(kMsg));
		assert(succeeded);
		return;
	}

	succeeded = PostMessage(kDlg, UWM_CHILDCREATE, NULL, reinterpret_cast<LPARAM>(hJobObject));
	assert(succeeded);

	wsprintf(kMsg, L"Process Running (%d).\n", pi.dwProcessId);
	succeeded = PostMessage(kDlg, UWM_UPDATEINFO, NULL, reinterpret_cast<LPARAM>(kMsg));
	assert(succeeded);

	succeeded = RegisterWaitForSingleObject(&hNewWaitHandle, pi.hProcess, WaitOrTimerCallback, hJobObject, INFINITE,
	                                        WT_EXECUTEONLYONCE);
	assert(succeeded);
}

void StopProcess()
{
	if (hJob)
	{
		BOOL succeeded = TerminateJobObject(hJob, -1);
		assert(succeeded);

		succeeded = CloseHandle(hJob);
		assert(succeeded);
	}
}

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);

void BuildCmdLine()
{
	const auto app_path = config.GetAppPath();
	const auto args = config.GetAppArgs();
	wsprintf(kCmdLine, L"%ls %ls", app_path, args);
}

int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine,
	int nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;
	config.Initialize();
	BuildCmdLine();

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
	const auto image_path = config.GetOffIconPath();
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
	const auto image_path = config.GetOffIconPath();

	if (const HMENU hMenu = GetSystemMenu(hWnd, FALSE))
	{
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T("About"));
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

	if (const HMENU hMenu = CreatePopupMenu())
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

void UpdateInfoText(LPCWSTR s)
{
	HWND wnd = GetDlgItem(kDlg, IDC_STATIC_INFO);
	SetWindowText(wnd, s);
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
			UpdateInfoText(kMsg);
			ShowWindow(hWnd, SW_RESTORE);
			break;

		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextMenu(hWnd);
		}

		break;

	case UWM_UPDATEINFO:
		UpdateInfoText(kMsg);
		break;

	case UWM_CHILDQUIT:
		if (hJob)
		{
			CloseHandle(hJob);
			hJob = NULL;
		}
		ShowNotificationData(false);
		swprintf_s(kMsg, L"Process Exit.");
		UpdateInfoText(kMsg);
		break;
	case UWM_CHILDCREATE:
		hJob = reinterpret_cast<HANDLE>(lParam);
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
		wmId = LOWORD(wParam);
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
