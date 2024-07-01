// Hooks to work around issues with the old version of AIL.
// A temporary hack until the game can use a more modern audio library.

#pragma once

#include "framework.h"

struct WaveHdrUser {
  char unknown1[20];
  unsigned int unknown2;
  int unknown3;
  unsigned int unknown4;
  char unknown5[40];
  int unknown6;
};

struct SomeTimerStruct {
  DWORD probablyState;
  void(__stdcall *callback)(DWORD);
  DWORD unknown;
  int accumulatedTime;
  int nextProcTime;
};

constexpr auto MAX_AIL_ALLOCATED_BLOCKS = 256;

typedef void(CALLBACK WaveOutProcFunc)(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                       DWORD_PTR dwParam2);
typedef void *(WINAPI AilFileReadFunc)(char *filename, void *dest);
typedef void(WINAPI AilMemFreeLock)(void *ptr);

class PatchedAil {
public:
  PatchedAil();
  ~PatchedAil();

private:
  HMODULE Module;

  // Original functions to replace
  static WaveOutProcFunc *OriginalWaveOutProc;
  static AilFileReadFunc *OriginalAilFileRead;
  static AilMemFreeLock *OriginalAilMemFreeLock;
  static LPTIMECALLBACK OriginalTimeProc;

  // Globals
  static volatile WAVEHDR **pLastFinishedWaveHdr;
  static volatile WaveHdrUser **pLastFinishedWaveHdrUser;
  static volatile unsigned int *pWaveOutProcGlobalThing;

  static volatile DWORD *pPeriod;
  static volatile DWORD *pCounter;
  static volatile SomeTimerStruct **pTimers;
  static volatile DWORD *pGlobal3;
  static volatile BOOL *pTimeProcLocked;
  static volatile DWORD *pGlobal5;
  static volatile DWORD *pNumTimers;

  // Hack to help mitigate the memory leaks while also preventing double frees
  static size_t AILAllocatedBlocks[MAX_AIL_ALLOCATED_BLOCKS];

  // Replacement functions
  static WaveOutProcFunc WaveOutProc;
  static AilFileReadFunc AilFileRead;
  static AilMemFreeLock AilMemFreeLock;
  static TIMECALLBACK TimeProc;
};
