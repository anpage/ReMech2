#include "PatchedShell.h"
#include "Remech2.h"

LoadMechVariantListFunc *PatchedShell::OriginalLoadMechVariantList;
CallsBitBlitFunc *PatchedShell::OriginalCallsBitBlit;
ShellDebugLogFunc *PatchedShell::OriginalShellDebugLog;

// Globals
volatile char *PatchedShell::MechVariantFilename;
volatile char (*PatchedShell::pMechVariantFilenames)[200][13];
void *PatchedShell::PrjObject;
volatile BOOL *PatchedShell::pBitBltResult;
volatile HDC *PatchedShell::pHdc;
volatile unsigned int *PatchedShell::pBitBlitWidth;
volatile unsigned int *PatchedShell::pBitBltHeight;
volatile HDC *PatchedShell::pHdcSrc;

// Required functions
LoadFileFromPrjFunc *PatchedShell::LoadFileFromPrj;

PatchedShell::PatchedShell() {
  if (Module != NULL) {
    DebugLog("ERROR: Multiple PatchedShell objects created?");
    exit(1);
  }

  Module = LoadLibraryA("MW2SHELL.DLL");
  if (Module == NULL) {
    exit(1);
  }

  MainProc = (ShellMainProc)GetProcAddress(Module, "ShellMain");
  if (MainProc == NULL) {
    exit(1);
  }

  ShellWindowProc = (WNDPROC)GetProcAddress(Module, "ShellWindowProc");
  if (ShellWindowProc == NULL) {
    exit(1);
  }

  size_t baseAddress = (size_t)Module;

  OriginalLoadMechVariantList = (LoadMechVariantListFunc *)(baseAddress + 0x0000c8b8);
  OriginalCallsBitBlit = (CallsBitBlitFunc *)(baseAddress + 0x00030ef9);
  OriginalShellDebugLog = (ShellDebugLogFunc *)(0x00017982 + baseAddress);

  MechVariantFilename = (char *)(baseAddress + 0x0007a800);
  pMechVariantFilenames = (char(*)[200][13])(baseAddress + 0x00079d80);
  PrjObject = (void *)(baseAddress + 0x00071230);

  LoadFileFromPrj = (LoadFileFromPrjFunc *)(baseAddress + 0x0002e346);

  pBitBltResult = (BOOL *)(baseAddress + 0x000965f4);
  pHdc = (HDC *)(baseAddress + 0x00066df8);
  pBitBlitWidth = (unsigned int *)(baseAddress + 0x00096e8cl);
  pBitBltHeight = (unsigned int *)(baseAddress + 0x00096e90);
  pHdcSrc = (HDC *)(baseAddress + 0x00067204);

  DetourTransactionBegin();
  DetourAttach((PVOID *)(&OriginalLoadMechVariantList), LoadMechVariantList);
  DetourAttach((PVOID *)(&OriginalCallsBitBlit), CallsBitBlit);
  DetourAttach((PVOID *)(&OriginalShellDebugLog), DebugLog);
  DetourAttach((PVOID *)(&TrueHeapFree), FakeHeapFree);
  DetourAttach((PVOID *)(&TrueRegCreateKeyExA), FakeRegCreateKeyExA);
  DetourAttach((PVOID *)(&TrueRegOpenKeyExA), FakeRegOpenKeyExA);
  DetourTransactionCommit();

  Ail = new PatchedAil();
}

PatchedShell::~PatchedShell() {
  DetourTransactionBegin();
  DetourDetach((PVOID *)(&OriginalLoadMechVariantList), LoadMechVariantList);
  DetourDetach((PVOID *)(&OriginalCallsBitBlit), CallsBitBlit);
  DetourDetach((PVOID *)(&OriginalShellDebugLog), DebugLog);
  DetourDetach((PVOID *)(&TrueHeapFree), FakeHeapFree);
  DetourDetach((PVOID *)(&TrueRegCreateKeyExA), FakeRegCreateKeyExA);
  DetourDetach((PVOID *)(&TrueRegOpenKeyExA), FakeRegOpenKeyExA);
  DetourTransactionCommit();
  FreeLibrary(Module);
  Module = NULL;
  delete Ail;
}

int PatchedShell::ShellMain(const char *introOrSim, HWND window) {
  DebugLog("Entering_Shell...\n");
  int result = MainProc(Module, 0, introOrSim, 1, window);
  DebugLog("Returned from Shell, retval=%d.\n", result);
  return result;
}

// Replacement function that uses cFilename instead of cAlternateFileName
void __cdecl PatchedShell::LoadMechVariantList(char *mechType) {
  // Clear the list
  memset((void *)*pMechVariantFilenames, 0, sizeof(*pMechVariantFilenames));

  // Make sure we at least have the default variant
  sprintf((char *)MechVariantFilename, "%s00std", mechType);
  strcpy((char *)(*pMechVariantFilenames)[0], (const char *)MechVariantFilename);

  // Load the built-in mech variants from the MW2.PRJ file into the first 100 indices
  for (int i = 1; i <= 99; i++) {
    sprintf((char *)MechVariantFilename, "%s%02dstd", mechType, i);
    int result = LoadFileFromPrj(PrjObject, nullptr, (char *)MechVariantFilename, 6);

    if (result > -1) {
      strcpy((char *)(*pMechVariantFilenames)[i], (const char *)MechVariantFilename);
    }
  };

  // Find all user-defined mech variants from the filesystem and load their names into index 100 and higher
  sprintf((char *)MechVariantFilename, "mek\\%s??usr.mek", mechType);
  _WIN32_FIND_DATAA findData;
  HANDLE hFindFile = FindFirstFileA((LPCSTR)MechVariantFilename, &findData);
  if (hFindFile != (HANDLE)-1) {
    BOOL foundNextFile;
    do {
      int i = (int)findData.cFileName[4] + (findData.cFileName[3] * 5 + -240) * 2 + 52;
      strncpy((char *)(*pMechVariantFilenames)[i], findData.cFileName, 8);
      (*pMechVariantFilenames)[i][8] = '\0';
      foundNextFile = FindNextFileA(hFindFile, &findData);
    } while (foundNextFile);
    FindClose(hFindFile);
  }
}

// Replacement function that doesn't assume BitBlt will assume the number of lines blitted
int __stdcall PatchedShell::CallsBitBlit() {
  *pBitBltResult = BitBlt(*pHdc, 0, 0, *pBitBlitWidth, *pBitBltHeight, *pHdcSrc, 0, 0, SRCCOPY);

  if (*pBitBltResult == FALSE) {
    DWORD lastError = GetLastError();
    DebugLog("GDI StretchBlt err: %d\n", lastError);
    return -1;
  }

  return 0;
}