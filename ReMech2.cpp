#include "framework.h"
#include "ReMech2.h"
#include "AIL.h"
#include "Shell.h"

static BOOL CheckReMech2Running(void)
{
	HWND hWnd = FindWindowA("REMECH 2", "REMECH 2");
	if (hWnd != NULL)
	{
		return TRUE;
	}

	return FALSE;
}

WNDPROC SimWindowProc = NULL;
WNDPROC ShellWindowProc = NULL;

enum ProcessType
{
	NONE,
	SIM,
	SHELL
};

static LRESULT __stdcall WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static ProcessType CurrentProcessType = NONE;

	switch (Msg)
	{
	case 0x41E:
		CurrentProcessType = NONE;
		break;
	case 0x41F:
		CurrentProcessType = SIM;
		break;
	case 0x420:
		CurrentProcessType = SHELL;
		break;
	}

	switch (CurrentProcessType)
	{
	case SIM:
		return SimWindowProc(hWnd, Msg, wParam, lParam);
	case SHELL:
		return ShellWindowProc(hWnd, Msg, wParam, lParam);
	default:
		return DefWindowProcA(hWnd, Msg, wParam, lParam);
	}
}

static HWND CreateGameWindow(HINSTANCE hInstance, int width, int height)
{
	static BOOL WindowCreated = FALSE;
	if (WindowCreated == FALSE) {
		WindowCreated = TRUE;
		WNDCLASSA wndClass{};
		wndClass.style = CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = WndProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = hInstance;
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = "REMECH 2";
		RegisterClassA(&wndClass);
	}

	tagRECT windowRect{};
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.right = width;
	windowRect.bottom = height;

	// Windowed
	DWORD dwStyle = 0xca0000;
	int displayWidth = GetSystemMetrics(SM_CXSCREEN);
	int displayHeight = GetSystemMetrics(SM_CYSCREEN);
	if ((width < displayWidth) || (height < displayHeight)) {
		AdjustWindowRect(&windowRect, dwStyle, NULL);
		windowRect.right = windowRect.right - windowRect.left;
		windowRect.bottom = windowRect.bottom - windowRect.top;
		windowRect.top = (displayHeight - windowRect.bottom) / 2;
		windowRect.left = (displayWidth - windowRect.right) / 2;
	}
	else {
		// Fullscreen
		dwStyle = WS_POPUP;
	}

	HWND window = CreateWindowExA(
		WS_EX_LEFT,
		"REMECH 2",
		"REMECH 2",
		dwStyle,
		windowRect.left, windowRect.top, windowRect.right, windowRect.bottom,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (window != NULL) {
		SetMenu(window, NULL);
		ShowWindow(window, SW_SHOWDEFAULT);
		UpdateWindow(window);
	}

	return window;
}

BOOL(WINAPI* TrueHeapFree)(HANDLE hHeap, DWORD dwFlags, _Frees_ptr_opt_ LPVOID lpMem) = HeapFree;

static BOOL WINAPI FakeHeapFree(HANDLE hHeap, DWORD dwFlags, _Frees_ptr_opt_ LPVOID lpMem)
{
	return TRUE;
}

typedef int(__stdcall* ShellMainProc)(HMODULE module, int always_0, const char* intro_or_sim, int always_1, HWND window);

static int StartShell(HWND window, const char* intro_or_sim, int always_1)
{
	HMODULE shellDll = LoadLibraryA("MW2SHELL.DLL");

	if (shellDll == NULL) {
		return 1;
	}

	ShellMainProc ShellMain = (ShellMainProc)GetProcAddress(shellDll, "ShellMain");
	if (ShellMain == NULL) {
		return 1;
	}

	ShellWindowProc = (WNDPROC)GetProcAddress(shellDll, "ShellWindowProc");
	if (ShellWindowProc == NULL) {
		return 1;
	}

	DetourTransactionBegin();
	DetourAttach((PVOID*)(&TrueLoadMechVariantList), (PVOID)FakeLoadMechVariantList);
	DetourAttach((PVOID*)(&TrueCallsBitBlit), (PVOID)FakeCallsBitBlit);
	DetourAttach((PVOID*)(&TrueWaveOutProc), FakeWaveOutProc);
	DetourAttach((PVOID*)(&TrueAilFileRead), FakeAilFileRead);
	DetourAttach((PVOID*)(&TrueAilMemFreeLock), FakeAilMemFreeLock);
	DetourAttach((PVOID*)(&TrueHeapFree), FakeHeapFree);
	DetourTransactionCommit();

	printf("Entering_Shell...");
	int result = ShellMain(shellDll, 0, intro_or_sim, always_1, window);
	printf("Returned from Shell, retval=%d.\n", result);
	FreeLibrary(shellDll);

	return result;
}

typedef int(__cdecl* SimMainProc)(HMODULE module, unsigned int always_0, LPSTR cmdline, void** param_4, int param_5, HWND param_6);

static int StartSim(HWND window, char* cmdline, void** param_3, int isNetGame)
{
	HMODULE simDll = LoadLibraryA("MW2.DLL");
	if (simDll == (HMODULE)0x0) {
		return 1;
	}

	SimMainProc SimMain = (SimMainProc)GetProcAddress(simDll, "SimMain");
	if (SimMain == NULL) {
		return 1;
	}

	SimWindowProc = (WNDPROC)GetProcAddress(simDll, "SimWindowProc");
	if (SimWindowProc == NULL) {
		return 1;
	}

	printf("Entering sim...");
	int result = (*SimMain)(simDll, 0, cmdline, param_3, isNetGame, window);
	printf("Returned from Sim, retval=%d.\n", result);
	FreeLibrary(simDll);

	return result;
}

typedef int (*NetMechLauncherProc)(void** param_1);

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	int retVal = 0;

	int net_selected = _strnicmp(lpCmdLine, "net", 3);
	if (net_selected == 0)
	{
		HMODULE netmechDll = LoadLibraryA("NETMECHW.DLL");
		if (netmechDll == NULL) {
			MessageBoxA(NULL, "NETMECHW.DLL is missing.", "REMECH 2", MB_ICONERROR);
			retVal = 1;
		}
		else
		{
			NetMechLauncherProc NetmechLauncher = (NetMechLauncherProc)GetProcAddress(netmechDll, "Launcher");
			if (NetmechLauncher == NULL)
			{
				MessageBoxA(NULL, "NETMECHW.DLL has not been installed properly.", "REMECH 2", MB_ICONERROR);
				retVal = 1;
			}
			else
			{
				char someNetmechStruct[28]; // TODO: Init the struct on the stack
				// skipped some struct stuff here
				while (true)
				{
					printf("Entering net DLL...\n");
					int netmechLauncherResult = NetmechLauncher((void**)(&someNetmechStruct));
					printf("Returned from net DLL.\n");
					if (netmechLauncherResult == 0) break;
					HWND gameWindow = CreateGameWindow(hInstance, 640, 480);
					retVal = StartSim(gameWindow, lpCmdLine, (void**)(&someNetmechStruct), 1);
					DestroyWindow(gameWindow);
					gameWindow = NULL;
					// skip
				}
				printf("Returned from net DLL.  Exiting...\n");
				FreeLibrary(netmechDll);
			}
		}
	}
	else
	{
		int cmdLineIsEmpty = _strcmpi(lpCmdLine, "");
		if (cmdLineIsEmpty == 0)
		{
			HMODULE netmechDll = GetModuleHandleA("NETMECHw.DLL");
			if (netmechDll != NULL) {
				FreeLibrary(netmechDll);
			}
			netmechDll = NULL;
			HWND gameWindow = CreateGameWindow(hInstance, 640, 480);
			retVal = StartShell(gameWindow, "intro", 1);
			do {
				if (retVal == -1) {
					MessageBoxA(
						NULL,
						"REMECH 2 is unable to locate necessary program components.",
						"REMECH 2",
						0x10
					);
					exit(1);
				}
				else if ((char)retVal == -1) {
					return retVal;
				}
				FILE* mw2PrmCfgFile;
				int result = fopen_s(&mw2PrmCfgFile, "mw2prm.cfg", "rb");
				if (mw2PrmCfgFile == NULL) {
					MessageBoxA(NULL, "Error opening MW2 parameter file.", "REMECH 2", MB_ICONERROR);
					exit(1);
				}
				char paramFileContents[536];
				size_t readResult = fread(paramFileContents, 536, 1, mw2PrmCfgFile);
				if (readResult != 1) {
					MessageBoxA(NULL, "Error reading MW2 parameter file.", "REMECH 2", MB_ICONERROR);
					exit(1);
				}
				fclose(mw2PrmCfgFile);
				int startSimResult = StartSim(gameWindow, (char*)(paramFileContents + 280), 0, 1);
				if (startSimResult == -1) {
					MessageBoxA(
						NULL,
						"REMECH 2 is unable to locate necessary program components.",
						"REMECH 2",
						MB_ICONERROR
					);
					exit(1);
				}
				else {
					retVal = startSimResult;
					if (retVal == -1) {
						return startSimResult;
					}
				}
				retVal = StartShell(gameWindow, "sim", 1);
			} while (true);
		}
	}

	HWND gameWindow = CreateGameWindow(hInstance, 640, 480);
	retVal = StartSim(gameWindow, lpCmdLine, 0, 1);

	return retVal;
}
