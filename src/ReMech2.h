#pragma once

#include "resource.h"
#include "framework.h"

extern WNDPROC SimWindowProc;
extern WNDPROC ShellWindowProc;

extern void DebugLog(const char *format, ...);

extern BOOL(WINAPI *TrueHeapFree)(HANDLE, DWORD, _Frees_ptr_opt_ LPVOID);
extern LSTATUS(WINAPI *TrueRegCreateKeyExA)(HKEY, LPCSTR, DWORD, LPSTR, DWORD, REGSAM, const LPSECURITY_ATTRIBUTES,
                                            PHKEY, LPDWORD);
extern LSTATUS(WINAPI *TrueRegOpenKeyExA)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY);

extern BOOL WINAPI FakeHeapFree(HANDLE hHeap, DWORD dwFlags, _Frees_ptr_opt_ LPVOID lpMem);
extern LSTATUS WINAPI FakeRegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions,
                                          REGSAM samDesired, const LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                          PHKEY phkResult, LPDWORD lpdwDisposition);
extern LSTATUS WINAPI FakeRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired,
                                        PHKEY phkResult);

extern MMRESULT(__stdcall *TrueTimeSetEvent)(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT);

extern MMRESULT __stdcall FakeTimeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK lpTimeProc, DWORD_PTR dwUser,
                                           UINT fuEvent);