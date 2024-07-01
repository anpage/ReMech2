// Replacement functions for the game's shell (main menu and such), contained in MW2SHELL.DLL

#pragma once

#include "framework.h"
#include "PatchedAil.h"

typedef int(__stdcall *ShellMainProc)(HMODULE module, int, const char *, int, HWND);

class PatchedShell {
public:
  PatchedShell();
  ~PatchedShell();
  int ShellMain(const char *introOrSim, HWND window);

private:
  HMODULE Module;
  ShellMainProc MainProc;
  PatchedAil *Ail;

  // Original functions to replace
  static void(__cdecl *OriginalLoadMechVariantList)(char *mechType);
  static int(__stdcall *OriginalCallsBitBlit)(void);
  static void(__cdecl *OriginalShellDebugLog)(const char *format, ...);

  // Globals
  static volatile char *MechVariantFilename;
  static volatile char (*pMechVariantFilenames)[200][13];
  static void *PrjObject;
  static volatile BOOL *pBitBltResult;
  static volatile HDC *pHdc;
  static volatile unsigned int *pBitBlitWidth;
  static volatile unsigned int *pBitBltHeight;
  static volatile HDC *pHdcSrc;

  // Required functions
  static int(__thiscall *LoadFileFromPrj)(void *_this, char *fileName, int something);

  // Replacement functions
  static void __cdecl LoadMechVariantList(char *mechType);
  static int __stdcall CallsBitBlit();
};
