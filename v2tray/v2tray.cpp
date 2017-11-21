﻿// v2tray.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "v2tray.h"
#include "v2ray.h"
#include <shellapi.h>
#include <stdio.h>
#include "WinHttp.h"

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "wininet.lib")

#define MAX_LOADSTRING 100
#define WM_ICON WM_USER + 1
#define MENU_RELOAD WM_USER + 2
#define MENU_EXIT WM_USER + 3
#define MENU_EDIT WM_USER + 4
#define MENU_PROXY WM_USER + 5
#define MENU_NOPROXY WM_USER + 6
#define MENU_SHOWCONSOLE WM_USER + 7
#define MENU_RUNATSTARTUP WM_USER + 8

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HANDLE m_hStartEvent;

NOTIFYICONDATA nid{};
NOTIFYICONDATA nid_off{};
V2Ray v2ray;


// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CheckOneInstance();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	size_t iPos = _tcsrchr(szPath, '\\') - szPath;
	szPath[iPos] = 0;
	SetCurrentDirectory(szPath);

	if (!CheckOneInstance())
	{
		return 1;
	}


    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_V2RAYTRAY, szWindowClass, MAX_LOADSTRING);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_V2RAYTRAY));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

BOOL CheckOneInstance()
{
	m_hStartEvent = CreateEventW(NULL, TRUE, FALSE, L"EVENT_NAME_HERE");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(m_hStartEvent);
		m_hStartEvent = NULL;
		// already exist
		// send message from here to existing copy of the application
		return FALSE;
	}
	// the only instance, start in a usual way
	return TRUE;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   WNDCLASS wndClass{};
   wndClass.hInstance = hInstance;
   wndClass.lpfnWndProc = &WndProc;
   wndClass.lpszClassName = szWindowClass;
   
   RegisterClass(&wndClass);

   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0, hInstance, 0);
   ShowWindow(hWnd, SW_HIDE);

   HICON hIco = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_V2RAYYO), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
   nid.cbSize = sizeof(nid);
   nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE;
   nid.hIcon = hIco;
   nid.hWnd = hWnd;
   wcscpy_s(nid.szTip, _T("V2rayYo"));
   nid.uVersion = NOTIFYICON_VERSION_4;
   nid.uCallbackMessage = WM_ICON;
   HICON hIco_off = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_V2RAYYO_OFF), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
   nid_off.cbSize = sizeof(nid_off);
   nid_off.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE;
   nid_off.hIcon = hIco_off;
   nid_off.hWnd = hWnd;
   wcscpy_s(nid_off.szTip, _T("V2rayYo"));
   nid_off.uVersion = NOTIFYICON_VERSION_4;
   nid_off.uCallbackMessage = WM_ICON;
   Shell_NotifyIcon(NIM_ADD, &nid);

   v2ray.init(hWnd, "v2ray.ini");

   return TRUE;
}

BOOL IsMyProgramRegisteredForStartup(TCHAR* pszAppName)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwRegType = REG_SZ;
	wchar_t szPathToExe[MAX_PATH] = {};
	DWORD dwSize = sizeof(szPathToExe);

	lResult = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_READ, &hKey);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		lResult = RegGetValueW(hKey, NULL, pszAppName, RRF_RT_REG_SZ, &dwRegType, szPathToExe, &dwSize);
		fSuccess = (lResult == 0);
	}

	if (fSuccess)
	{
		fSuccess = (_tcslen(szPathToExe) > 0) ? TRUE : FALSE;
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

BOOL RegisterMyProgramForStartup(TCHAR* pszAppName, TCHAR* pathToExe, TCHAR* args, BOOL bAdd = TRUE)
{
	HKEY hKey = NULL;
	LONG lResult = 0;
	BOOL fSuccess = TRUE;
	DWORD dwSize;

	const size_t count = MAX_PATH * 2;
	wchar_t szValue[count] = {};


	_tcscpy_s(szValue, count, L"\"");
	_tcscat_s(szValue, count, pathToExe);
	_tcscat_s(szValue, count, L"\" ");

	if (args != NULL)
	{
		// caller should make sure "args" is quoted if any single argument has a space
		// e.g. (L"-name \"Mark Voidale\"");
		_tcscat_s(szValue, count, args);
	}

	lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &hKey, NULL);

	fSuccess = (lResult == 0);

	if (fSuccess)
	{
		if (bAdd)
		{
			dwSize = (_tcslen(szValue) + 1) * 2;
			lResult = RegSetValueEx(hKey, pszAppName, 0, REG_SZ, (BYTE*)szValue, dwSize);
			fSuccess = (lResult == 0);
		}
		else
		{
			lResult = RegDeleteValue(hKey, pszAppName);
			fSuccess = (lResult == 0);
		}
	}

	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return fSuccess;
}

void popMenu(HINSTANCE hInst, HWND hWnd)
{
	MENUITEMINFO separatorBtn = { 0 };
	separatorBtn.cbSize = sizeof(MENUITEMINFO);
	separatorBtn.fMask = MIIM_FTYPE;
	separatorBtn.fType = MFT_SEPARATOR;

	HMENU hMenu = CreatePopupMenu();
	if (hMenu)
	{
		InsertMenu(hMenu, -1, MF_BYPOSITION, MENU_EXIT, _T("退出"));

		POINT pt;
		GetCursorPos(&pt);
		SetForegroundWindow(hWnd);
		TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
		PostMessage(hWnd, WM_NULL, 0, 0);

		DestroyMenu(hMenu);
	}
}

void showConsole()
{
	if (AllocConsole())
	{
		freopen("CONOUT$", "w", stdout);
	}
	else
	{
		HWND hWnd = GetConsoleWindow();
		ShowWindow(hWnd, IsWindowVisible(hWnd) ? SW_HIDE : SW_SHOW);
		SetForegroundWindow(hWnd);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
				break;
			case MENU_EXIT:
			{
				v2ray.setNoProxy();
				DestroyWindow(hWnd);
				break;
			}
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_ICON:
	{
		int nmId = LOWORD(lParam);
		// 分析菜单选择: 
		switch (nmId)
		{
		case WM_CONTEXTMENU:
			DestroyWindow(hWnd);
			break;
		case WM_RBUTTONUP:
			popMenu(hInst, hWnd);
			break;
		case WM_LBUTTONUP:
			if (v2ray.Return_switch_proxy())
			{
				Shell_NotifyIcon(NIM_MODIFY, &nid_off);
				v2ray.setNoProxy();
			}
			else
			{
				Shell_NotifyIcon(NIM_MODIFY, &nid);
				v2ray.setProxy();
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
	}
    return 0;
}