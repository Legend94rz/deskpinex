#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT __declspec(dllexport)
#endif
#define WM_MYMSG WM_USER+2
#define WM_SELWND WM_USER+3
#define NIM_NOTIFY WM_USER+4
#define IDT_TIMER 10001
#define GWL_HINSTANCE       (-6)
#define GWL_WNDPROC         (-4)
#define GWL_USERDATA        (-21)
#define wstrMODULE TEXT("DeskPinsHook")
#define wstrMODULE64 TEXT("DeskPinsHook_x64")
EXPORT BOOL CALLBACK InstallMouseHook(HWND);
EXPORT BOOL CALLBACK UnstallMouseHook(HWND);
EXPORT int CALLBACK GetMode();
EXPORT void CALLBACK SetMode(int);
