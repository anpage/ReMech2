#include "ReMech2.h"
#include "PatchedShell.h"
#include "PatchedSim.h"
#include <memory>

// Win32 shims
BOOL(WINAPI *TrueHeapFree)(HANDLE, DWORD, _Frees_ptr_opt_ LPVOID) = HeapFree;
BOOL WINAPI FakeHeapFree(HANDLE hHeap, DWORD dwFlags, _Frees_ptr_opt_ LPVOID lpMem) { return TRUE; }

LSTATUS(WINAPI *TrueRegCreateKeyExA)
(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, const LPSECURITY_ATTRIBUTES, PHKEY, LPDWORD) = RegCreateKeyExA;
LSTATUS WINAPI FakeRegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions,
                                   REGSAM samDesired, const LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult,
                                   LPDWORD lpdwDisposition) {
  if (hKey == HKEY_LOCAL_MACHINE) {
    hKey = HKEY_CURRENT_USER;
  }
  LSTATUS retVal = TrueRegCreateKeyExA(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes,
                                       phkResult, lpdwDisposition);
  return retVal;
}

LSTATUS(WINAPI *TrueRegOpenKeyExA)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY) = RegOpenKeyExA;
LSTATUS WINAPI FakeRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) {
  if (hKey == HKEY_LOCAL_MACHINE) {
    hKey = HKEY_CURRENT_USER;
  }
  LSTATUS retVal = TrueRegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired, phkResult);
  return retVal;
}

void DebugLog(const char *format, ...) {
  char msgBuf[256];
  va_list args;
  va_start(args, format);
  vsnprintf(msgBuf, 256, format, args);
  va_end(args);
  OutputDebugStringA(msgBuf);
}

DWORD GetLastErrorAsString(LPSTR buffer, DWORD size) {
  DWORD errorMessageID = GetLastError();
  return FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, size, NULL);
}

WNDPROC SimWindowProc = NULL;
WNDPROC ShellWindowProc = NULL;

enum ProcessType { NONE, SIM, SHELL };

static LRESULT __stdcall WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
  static ProcessType CurrentProcessType = NONE;

  switch (Msg) {
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

  switch (CurrentProcessType) {
  case SIM:
    return SimWindowProc(hWnd, Msg, wParam, lParam);
  case SHELL:
    return ShellWindowProc(hWnd, Msg, wParam, lParam);
  default:
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
  }
}

static HWND CreateGameWindow(HINSTANCE hInstance, int width, int height) {
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
  } else {
    // Fullscreen
    dwStyle = WS_POPUP;
  }

  HWND window = CreateWindowExA(WS_EX_LEFT, "REMECH 2", "REMECH 2", dwStyle, windowRect.left, windowRect.top,
                                windowRect.right, windowRect.bottom, NULL, NULL, hInstance, NULL);

  if (window != NULL) {
    SetMenu(window, NULL);
    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);
  }

  return window;
}

static int StartShell(HWND window, const char *introOrSim) {
  std::unique_ptr<PatchedShell> shell = std::unique_ptr<PatchedShell>(new PatchedShell());
  return shell->ShellMain(introOrSim, window);
}

static int StartSim(HWND window, char *cmdLine, void **unknown, int isNetGame) {
  std::unique_ptr<PatchedSim> shell = std::unique_ptr<PatchedSim>(new PatchedSim());
  return shell->SimMain(cmdLine, unknown, isNetGame, window);
}

void CALLBACK TimeCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
  DebugLog("Time callback!");
  return;
}

MMRESULT(__stdcall *TrueTimeSetEvent)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT) = timeSetEvent;

MMRESULT __stdcall FakeTimeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser,
                                    UINT fuEvent) {
  return TrueTimeSetEvent(uDelay, uResolution, lpTimeProc, dwUser, fuEvent);
}

typedef int (*NetMechLauncherProc)(void **param_1);

static char CdDriveLetter = '\0';

static char CdCheck() {
  if (CdDriveLetter == '\0') {
    LPSTR driveStrings = (LPSTR)malloc(sizeof(TCHAR) * 105);
    GetLogicalDriveStringsA(104, driveStrings);

    char filenameBuf[20];
    strcpy(filenameBuf, " :\\OLD_HERC.DRV");

    // Find a CD with the file "OLD_HERC.DRV"
    LPCSTR driveName;
    for (driveName = driveStrings; *driveName != '\0'; driveName = driveName + 4) {
      UINT driveType = GetDriveTypeA(driveName);
      if (driveType == DRIVE_CDROM) {
        filenameBuf[0] = driveName[0];
        WIN32_FIND_DATAA findData;
        HANDLE fileHandle = FindFirstFileA(filenameBuf, &findData);
        if (fileHandle != (HANDLE)-1) {
          FindClose(fileHandle);
          break;
        }
      }
    }
    if (*driveName != '\0') {
      CdDriveLetter = *driveName;
    }
    free(driveStrings);
  }
  return CdDriveLetter;
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
                     _In_ int nCmdShow) {
  int retVal = 0;

  int net_selected = _strnicmp(lpCmdLine, "net", 3);
  if (net_selected == 0) {
    HMODULE netmechDll = LoadLibraryA("NETMECHW.DLL");
    if (netmechDll == NULL) {
      MessageBoxA(NULL, "NETMECHW.DLL is missing.", "REMECH 2", MB_ICONERROR);
      retVal = 1;
    } else {
      NetMechLauncherProc NetmechLauncher = (NetMechLauncherProc)GetProcAddress(netmechDll, "Launcher");
      if (NetmechLauncher == NULL) {
        MessageBoxA(NULL, "NETMECHW.DLL has not been installed properly.", "REMECH 2", MB_ICONERROR);
        retVal = 1;
      } else {
        char someNetmechStruct[28]; // TODO: Init the struct on the stack
        // skipped some struct stuff here
        while (true) {
          DebugLog("Entering net DLL...\n");
          int netmechLauncherResult = NetmechLauncher((void **)(&someNetmechStruct));
          DebugLog("Returned from net DLL.\n");
          if (netmechLauncherResult == 0)
            break;
          HWND gameWindow = CreateGameWindow(hInstance, 640, 480);
          retVal = StartSim(gameWindow, lpCmdLine, (void **)(&someNetmechStruct), 1);
          DestroyWindow(gameWindow);
          gameWindow = NULL;
          // skip
        }
        DebugLog("Returned from net DLL.  Exiting...\n");
        FreeLibrary(netmechDll);
      }
    }
  } else {
    int cmdLineIsEmpty = _strcmpi(lpCmdLine, "");
    if (cmdLineIsEmpty == 0) {
      char cdDriveLetter = CdCheck();
      if (cdDriveLetter == '\0') {
        MessageBoxA((HWND)0x0, "You must insert the game's CD into your CD-ROM drive.", "REMECH 2", MB_ICONERROR);
        exit(1);
      }
      HMODULE netmechDll = GetModuleHandleA("NETMECHW.DLL");
      if (netmechDll != NULL) {
        FreeLibrary(netmechDll);
      }
      netmechDll = NULL;
      HWND gameWindow = CreateGameWindow(hInstance, 640, 480);
      retVal = StartShell(gameWindow, "intro");
      do {
        if (retVal == -1) {
          MessageBoxA(NULL, "REMECH 2 is unable to locate necessary program components.", "REMECH 2", MB_ICONERROR);
          exit(1);
        } else if ((char)retVal == -1) {
          return retVal;
        }
        FILE *mw2PrmCfgFile;
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
        int startSimResult = StartSim(gameWindow, (char *)(paramFileContents + 280), 0, 1);
        if (startSimResult == -1) {
          MessageBoxA(NULL, "REMECH 2 is unable to locate necessary program components.", "REMECH 2", MB_ICONERROR);
          exit(1);
        } else {
          retVal = startSimResult;
          if (retVal == -1) {
            return startSimResult;
          }
        }
        retVal = StartShell(gameWindow, "sim");
      } while (true);
    }
  }

  HWND gameWindow = CreateGameWindow(hInstance, 640, 480);
  retVal = StartSim(gameWindow, lpCmdLine, 0, 1);

  return retVal;
}
