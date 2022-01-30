#ifndef _WIN64
#pragma comment(lib,"DeskPinsHook.lib")
#else
#pragma comment(lib,"DeskPinsHook_x64.lib")
#endif
#pragma comment(linker, "/MANIFESTDEPENDENCY:\"type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595B64144CCF1DF'\"")

#include <Windows.h>
#include <process.h>
#include <CommCtrl.h>
#include "NewHead.h"
#include "resource.h"

const int cxTrackBar=150,cyTrackBar=40;

NOTIFYICONDATA nic;//托盘图标
HFONT hFont;
HHOOK hHook;
HWND hLastWnd;
int iCountPins;
HINSTANCE hInstance;
HWND hPins[256];
HWND WINAPI CreateTrackBar(HWND, UINT, UINT);
POINT* CalculatePinRect(HWND hParent)
{
	RECT rtWndRect;
	POINT* pPt = new POINT();
	GetWindowRect(hParent, &rtWndRect);
	int iTemp = 10 + GetSystemMetrics(SM_CXSIZE) + GetSystemMetrics(SM_CXFRAME);//边框+关闭
	if (GetWindowLong(hParent, GWL_STYLE)&WS_MAXIMIZE)iTemp += GetSystemMetrics(SM_CXSIZE);
	if (GetWindowLong(hParent, GWL_STYLE)&WS_MINIMIZEBOX)iTemp += GetSystemMetrics(SM_CXSIZE);
	pPt->y = rtWndRect.top+5;
	pPt->x = rtWndRect.right - iTemp - 2*GetSystemMetrics(SM_CYCAPTION);			//正方形区域
	return pPt;
}
LRESULT CALLBACK ChildProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			{
				SetTimer(hwnd, IDT_TIMER, 25, NULL);
				SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
				COLORREF clTransparent = RGB(255, 255, 255);
				SetLayeredWindowAttributes(hwnd, clTransparent, 0, LWA_COLORKEY);
			}
			return 0;
		case WM_TIMER:
			{
				POINT* pPt = CalculatePinRect(GetWindow(hwnd, GW_OWNER));
				SetWindowPos(hwnd, HWND_TOP, pPt->x, pPt->y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
				delete pPt;
			}
			return 0;
		case WM_LBUTTONDOWN:
		//	SendMessage(hwnd, WM_DESTROY,0,0);
		//return 0;
		case WM_DESTROY:
			KillTimer(hwnd, IDT_TIMER);
			{
				HWND Owner=GetWindow(hwnd,GW_OWNER);
				SetWindowPos(Owner, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				SetWindowLong(Owner, GWL_EXSTYLE, GetWindowLong(Owner, GWL_EXSTYLE) ^ WS_EX_LAYERED);
			}
			_endthread();
			PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
void ThreadNewPin(void* pVoid)
{
	HWND hDes = (HWND)pVoid;
	MSG msg;
	HINSTANCE hInst = (HINSTANCE)GetWindowLong(hDes, GWL_HINSTANCE);
	HWND hTemp = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,TEXT("PINS"), NULL, WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS ,
		0, 0, 16, 16,hDes,NULL,hInst, 0);
	hPins[iCountPins++] = hTemp;
	SetWindowPos(hDes, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	while (GetMessage(&msg, hTemp, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
	static int cxScreen, cyScreen;
	static HWND hDes, hOldWnd,hBar;
	static HWND hStatic;
	static RECT rtPinRect;
	static HMODULE hDLL;
	static HMENU hRMenu;
	POINT pt;
	switch (message)
	{
	case WM_CREATE:
	{
		cxScreen = GetSystemMetrics(SM_CXSCREEN);
		cyScreen = GetSystemMetrics(SM_CYSCREEN);
		hBar=CreateTrackBar(hwnd, 100,255);
		//MoveWindow(hwnd, cxScreen / 3, cyScreen / 3, cxScreen / 3, cyScreen / 3, TRUE);
		//HWND hBut = CreateWindow(WC_BUTTON, TEXT("捕捉窗口"), WS_CHILD | WS_VISIBLE, 150, 10, 75, 25, hwnd, (HMENU)1, hInst, 0);
		//hStatic = CreateWindow(WC_STATIC, NULL, WS_CHILD | WS_VISIBLE, 10, 55, 200, 25, hwnd, (HMENU)2, hInst, 0);
		//SendMessage(hBut, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		//SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
		hRMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU));
		hRMenu = GetSubMenu(hRMenu, 0);
	}
	return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDM_REMOVE:
				for (int i = 0; i < iCountPins; i++)
					SendMessage(hPins[i], WM_DESTROY, 0, 0);
				iCountPins = 0; hLastWnd = NULL;
			break;
			case IDM_EXIT:
				for (int i = 0; i < iCountPins; i++)
					 SendMessage(hPins[i], WM_DESTROY, 0, 0);
				SendMessage(hwnd, WM_DESTROY, 0, 0);
			break;
			case IDM_TRANS:
			{
				POINT pt;
				GetCursorPos(&pt);
				MoveWindow(hwnd, pt.x - cxTrackBar/2, pt.y - cyTrackBar/2, cxTrackBar+2, cyTrackBar+2,TRUE);
				SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				ShowWindow(hwnd, SW_SHOWDEFAULT);
			}
			break;
			case IDM_SELWND:
				SendMessage(hwnd, NIM_NOTIFY, 0, WM_LBUTTONDOWN);
				break;
		}
	return 0;
	case WM_MYMSG:
		//注掉~
		if (wParam == 1)//移动
		{
			SetWindowText(hStatic, (LPTSTR)lParam);
		}
		else
		{
			SetWindowText(hStatic, (LPTSTR)lParam);
		}
	return 0;
	case WM_ACTIVATE:
		if (LOWORD(wParam) == (WPARAM)WA_INACTIVE)
			ShowWindow(hwnd, SW_HIDE);
		return 0;
	case WM_SELWND:
		{
			int idThread;
			hLastWnd = (HWND)lParam;
			SetWindowLong((HWND)lParam, GWL_EXSTYLE, GetWindowLong((HWND)lParam, GWL_EXSTYLE) | WS_EX_LAYERED);
			//SetWindowLong((HWND)lParam, GWL_STYLE,   GetWindowLong((HWND)lParam,GWL_STYLE)| WS_OVERLAPPEDWINDOW);//for resize Mathematica
			_beginthread(ThreadNewPin, 0, (LPVOID)lParam);
		}
	return 0;
	case WM_HSCROLL:
		if (LOWORD(wParam) == SB_THUMBTRACK)
		{
			SetLayeredWindowAttributes(hLastWnd, 0, HIWORD(wParam), LWA_ALPHA);
		}
	return 0;
	case WM_DESTROY:
		UnstallMouseHook(hwnd);
		Shell_NotifyIcon(NIM_DELETE, &nic);
		PostQuitMessage(0);
	return 0;
	case NIM_NOTIFY:
		switch (lParam)
		{
			case WM_LBUTTONDOWN:
				SetMode(1);
				if (!InstallMouseHook(hwnd)){ MessageBox(hwnd, TEXT("安装钩子失败，程序退出！"), TEXT("错误"), MB_OK); ExitProcess(0); }
				break;
			case WM_RBUTTONDOWN:
				GetCursorPos(&pt);
				SetForegroundWindow(hwnd);
				TrackPopupMenu(hRMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				break;
		}
	return 0;
	case WM_HOTKEY:
		if (LOWORD(lParam) == 0 && HIWORD(lParam) == VK_F11)
		{
			SendMessage(hwnd, NIM_NOTIFY, 0, WM_LBUTTONDOWN);
		}
	return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
HWND WINAPI CreateTrackBar(HWND hOwner, UINT iMin, UINT iMax)
{
	HWND hBar = CreateWindowEx(0, TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE | TBS_ENABLESELRANGE | TBS_AUTOTICKS, 
		1,1, cxTrackBar,cyTrackBar,hOwner, 0,::hInstance, 0);
	SendMessage(hBar, TBM_SETRANGE, TRUE,MAKELONG(iMin, iMax));
	SendMessage(hBar, TBM_SETTICFREQ, 10, 0);
	SendMessage(hBar, TBM_SETPOS, TRUE, iMax);
	return hBar;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR szCmdLine, int iCmdShow)
{
	HWND hwnd;
	WNDCLASS wndclass,wcPin;
	MSG msg;
	::hInstance = hInstance;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hbrBackground = (HBRUSH)(BLACK_BRUSH);
	wndclass.hCursor       = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hInstance     = hInstance;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.lpszClassName = TEXT("MainWindow");
	wndclass.lpszMenuName  = NULL;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;

	if (FindWindow(wndclass.lpszClassName, NULL))
	{
		MessageBox(NULL, TEXT("DeskPinsEx已在运行！"), TEXT("提示"), MB_OK);
		ExitProcess(0);
	}
	
//=============================================================
	HBITMAP hBitmap = LoadBitmap(hInstance,MAKEINTRESOURCE( IDB_PINIMG));

	wcPin.cbClsExtra       = 0;
	wcPin.cbWndExtra       = 0;
	wcPin.hbrBackground    = CreatePatternBrush(hBitmap);
	wcPin.hCursor          = LoadCursor(hInstance,MAKEINTRESOURCE( IDC_HITCUR));
	wcPin.hIcon            = NULL;
	wcPin.hInstance        = 0;					//Warning!!;
	wcPin.lpfnWndProc      = ChildProc;
	wcPin.lpszClassName    = TEXT("PINS");
	wcPin.lpszMenuName     = NULL;
	wcPin.style            = 0;

	hFont = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, FF_SWISS, L"Microsoft Sans Serif");

	RegisterClass(&wndclass);
	RegisterClass(&wcPin);
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST,TEXT("MainWindow"), TEXT("DeskPins"), WS_POPUP,
		0, 0, 0, 0,
		NULL, NULL, hInstance, 0);

	INITCOMMONCONTROLSEX ccex;
	ccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ccex.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&ccex);

	nic.cbSize = sizeof(nic);
	nic.hWnd = hwnd;
	nic.uID = 1;
	nic.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON));
	lstrcpy(nic.szTip, TEXT("DeskPinsEx"));
	nic.uCallbackMessage = NIM_NOTIFY;
	nic.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	if (!RegisterHotKey(hwnd, 1, MOD_CONTROL, VK_F12) || !RegisterHotKey(hwnd,2,0,VK_F11))
		MessageBox(NULL, TEXT("注册热键失败！"), TEXT("错误"), MB_OK);

	//ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	Shell_NotifyIcon(NIM_ADD, &nic);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
