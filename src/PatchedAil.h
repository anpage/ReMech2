// Hooks to work around issues with the old version of AIL.
// A temporary hack until the game can use a more modern audio library.

#pragma once

#include "framework.h"

struct WaveHdrUser {
  char unknown1[20];
  uint32_t unknown2;
  int32_t unknown3;
  uint32_t unknown4;
  char unknown5[40];
  int32_t unknown6;
};

typedef void(__stdcall AilCallback)(uint32_t);

struct SomeTimerStruct {
  uint32_t probablyState;
  AilCallback *callback;
  uint32_t unknown;
  int32_t accumulatedTime;
  int32_t nextProcTime;
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
