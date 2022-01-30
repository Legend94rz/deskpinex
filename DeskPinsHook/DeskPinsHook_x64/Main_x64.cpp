#include <Windows.h>
#include "NewHead.h"
#include "resource.h"
#pragma data_seg("shared")
	HWND g_hwnd;
	HHOOK g_hHook64;
	HHOOK g_hGMSGHook64;
	HHOOK g_hPaintHook64;
	HINSTANCE g_hInst;
	HINSTANCE g_HookInst;
	WNDPROC g_oldProc;
	int iMode = 0;
	bool IsMaped = FALSE;
#pragma data_seg()
int WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	g_HookInst = hInstance;
	return TRUE;
}
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSLLHOOKSTRUCT* mls;
	mls = (MSLLHOOKSTRUCT*)lParam;
	HWND hDes;
	static TCHAR Buffer[128];
	switch (wParam)
	{
		case WM_MOUSEMOVE:
		{
			hDes = WindowFromPoint(mls->pt);
			GetWindowText(hDes, Buffer, 64);
			SendMessage(g_hwnd, WM_MYMSG, (WPARAM)1, (LPARAM)Buffer);
			//SetCapture(GetDesktopWindow());
			//HCURSOR hCursor = LoadCursor(g_HookInst, MAKEINTRESOURCE(IDC_SELCUR));
			//SetCursor(hCursor);
			//ReleaseCapture();
		}
		break;
		case WM_LBUTTONDOWN:
			hDes = WindowFromPoint(mls->pt);
			while (GetParent(hDes))hDes = GetParent(hDes);
			if (!(GetWindowLong(hDes, GWL_EXSTYLE)&WS_EX_TOPMOST))
				SendMessage(g_hwnd, WM_SELWND, 0, (LPARAM)hDes);
		case WM_RBUTTONDOWN:
			if (GetMode() == 1)
			{
				UnstallMouseHook(g_hwnd);
				SetMode(0);
			}
		break;
	}
	return CallNextHookEx(g_hHook64, nCode, wParam, lParam);
}
EXPORT BOOL CALLBACK InstallMouseHook(HWND hwnd)
{
	g_hwnd = hwnd;
	g_hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
	g_hHook64 = SetWindowsHookEx(WH_MOUSE_LL, HookProc, GetModuleHandle(wstrMODULE64), 0);
	if (!g_hHook64)return FALSE;
	return TRUE;
}
EXPORT BOOL CALLBACK UnstallMouseHook(HWND hwnd)
{
	if (hwnd != g_hwnd || !hwnd)return FALSE;
	BOOL bUnhook = UnhookWindowsHookEx(g_hHook64);
	if (bUnhook)g_hwnd = NULL;
	return bUnhook;
}
EXPORT int CALLBACK GetMode()
{
	return iMode;
}
EXPORT void CALLBACK SetMode(int x)
{
	iMode = x;
}
