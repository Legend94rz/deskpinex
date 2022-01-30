#include <Windows.h>
#include "NewHead.h"
#include "resource.h"
#pragma data_seg("shared")
	HWND g_hwnd;
	HHOOK g_hHook;
	HHOOK g_hGMSGHook;
	HHOOK g_hPaintHook;
	HINSTANCE g_hInst;
	HINSTANCE g_HookInst;
	WNDPROC g_oldProc;
	int iMode = 0;
#pragma data_seg()
int EnableDebugPriv(const WCHAR *name)
{
	BOOL retn;
	HANDLE hToken;
	retn = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
	if (retn != TRUE)
	{
		//cout << "获取令牌句柄失败!" << endl;
		return -1;
	}

	TOKEN_PRIVILEGES tp; //新特权结构体    
	LUID Luid;
	retn = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid);

	if (retn != TRUE)
	{
		//cout << "获取Luid失败" << endl;
		return -1;
	}
	//给TP和TP里的LUID结构体赋值    
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = Luid;

	AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		//cout << "修改特权不完全或失败!" << endl;
	}
	else
	{
		//cout << "修改成功!" << endl;
	}
	return 1;
}
//LRESULT CALLBACK newWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
//{
//	switch (message)
//	{
//		case WM_LBUTTONDOWN:
//			MessageBeep(0);
//		break;
//	}
//	return CallWindowProc(g_oldProc,hwnd,message,wParam,lParam);
//}
int WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	g_HookInst = hInstance;
	return TRUE;
}
//LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
//{
//	MSLLHOOKSTRUCT* mls;
//	mls=(MSLLHOOKSTRUCT*)lParam;
//	HWND hDes;
//	static TCHAR Buffer[128];
//	switch (wParam)
//	{
//		case WM_MOUSEMOVE:
//		{
//			hDes = WindowFromPoint(mls->pt);
//			GetWindowText(hDes,Buffer,64);
//			SendMessage(g_hwnd, WM_MYMSG, (WPARAM)1, (LPARAM)Buffer);
//			//HCURSOR hCursor = LoadCursor(g_HookInst, MAKEINTRESOURCE(IDC_SELCUR));
//			//SetCursor(hCursor);
//		}
//		break;
//		case WM_LBUTTONDOWN:
//		{
//			hDes = WindowFromPoint(mls->pt);
//			GetWindowText(hDes, Buffer, 64);
//			SetWindowPos(hDes, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
//			HDC hdc=GetWindowDC(hDes);
//			HPEN hPen=CreatePen(PS_SOLID,3,RGB(255,0,0));
//			SelectObject(hdc,hPen);
//
//			//Todo:画大头针
//			Rectangle(hdc, 0, 0, 125, 90);
//
//			/*注入Dll*/
//			//DWORD dwPID=GetWindowThreadProcessId(hDes,NULL);
//			//if (dwPID==0)
//			//	MessageBox(NULL,TEXT("PID=0"),TEXT("warning"),MB_OK);
//			//if (EnableDebugPriv(SE_DEBUG_NAME)<0)
//			//	MessageBox(NULL,L"提升权限失败",L"warn0",MB_OK);
//			//HANDLE hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwPID);
//			//if (hProcess==NULL)
//			//{
//			//	MessageBox(NULL,TEXT("进程打开失败"),TEXT("错误3"),MB_OK);
//			//	goto FINISH;
//			//}
//			//LPTHREAD_START_ROUTINE pRemoteStart=(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("kernel32")),"LoadLibraryA");
//			//if (pRemoteStart==NULL){
//			//	MessageBox(NULL, TEXT("远程进程创建失败！"), TEXT("错误2"), MB_OK); goto FINISH;
//			//}
//			//TCHAR szDllpath[260];
//			//GetCurrentDirectory(250,szDllpath);
//			//lstrcat(szDllpath,TEXT("\\DllToInsert.dll"));
//			//LPVOID pRemotePath=VirtualAllocEx(hProcess,NULL,lstrlen(szDllpath)+1,MEM_COMMIT,PAGE_READWRITE);
//			//if (pRemotePath==NULL){
//			//	MessageBox(NULL, TEXT("无法分配内存"), TEXT("错误"), MB_OK); goto FINISH;
//			//}
//			//WriteProcessMemory(hPen,pRemotePath,szDllpath,lstrlen(szDllpath)+1,NULL);
//			//DWORD dwThreadID;
//			//HANDLE hThread=CreateRemoteThread(hProcess,0,0,pRemoteStart,pRemotePath,0,&dwThreadID);
//			//FINISH:
//
//			DeleteObject(hPen);
//			ReleaseDC(hDes,hdc);
//			SendMessage(g_hwnd, WM_MYMSG, (WPARAM)2,(LPARAM)Buffer);
//
//			UnstallMouseHook(g_hwnd);
//		}
//		break;
//	}
//	return CallNextHookEx(g_hHook64, nCode, wParam, lParam);
//}
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	//EVENTMSG* mls;
	//mls = (EVENTMSG*)lParam;
	//switch( mls->message )
	//{
	//	case WM_MOUSEMOVE:
	//	{
	//		MessageBox(NULL,L"OKOKOK",L"OKOK",MB_OK);
	//	}
	//	break;
	//}
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}
LRESULT CALLBACK GetMsgHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG * msg;
	msg = (MSG*)lParam;
	switch (msg->message)
	{
		case WM_NCMOUSEMOVE:case WM_NCLBUTTONDOWN:
			//MessageBeep(0);
			break;
	}
	return CallNextHookEx(g_hGMSGHook, nCode, wParam, lParam);
}
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT* cwp;
	cwp = (CWPSTRUCT*)lParam;
	switch (cwp->message)
	{
		case WM_MOVE:case WM_SIZE:
			//MessageBeep(0);
			break;
		case WM_NCHITTEST:
			//if (iMode == 1)
			//	MessageBeep(0);
			break;
	}
	return CallNextHookEx(g_hPaintHook, nCode, wParam, lParam);
}
EXPORT BOOL CALLBACK InstallMouseHook(HWND hwnd)
{
	g_hwnd = hwnd;
	g_hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
	g_hHook = SetWindowsHookEx(WH_MOUSE_LL, HookProc, GetModuleHandle(wstrMODULE), 0);
	if (!g_hHook)return FALSE;
	return TRUE;
}
EXPORT BOOL CALLBACK UnstallMouseHook(HWND hwnd)
{
	if (hwnd != g_hwnd || !hwnd)return FALSE;
	BOOL bUnhook = UnhookWindowsHookEx(g_hHook);
	if (bUnhook)g_hwnd = NULL;
		return bUnhook;
}
EXPORT BOOL CALLBACK InstallMsgHook(HWND hwnd)
{
	g_hwnd = hwnd;
	g_hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
	g_hGMSGHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgHookProc, GetModuleHandle(wstrMODULE), 0);
	g_hPaintHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, GetModuleHandle(wstrMODULE), 0);
	if (!g_hGMSGHook || !g_hPaintHook)return FALSE;
	return TRUE;
}
EXPORT BOOL CALLBACK UnstallGetMsgHook(HWND hwnd)
{
	if (hwnd != g_hwnd || !hwnd)return FALSE;
	BOOL bUnhook = UnhookWindowsHookEx(g_hGMSGHook)
		&& UnhookWindowsHookEx(g_hPaintHook);
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
