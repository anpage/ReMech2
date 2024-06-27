// Hooks to work around issues with the old version of AIL.
// A temporary hack until the game can use a more modern audio library.

#pragma once

#include "framework.h"

struct WaveHdrUser {
  char unknown_1[20];
  unsigned int field_1;
  int field_2;
  unsigned int field_3;
  char unknown_2[40];
  int field_4;
};

constexpr auto MAX_AIL_ALLOCATED_BLOCKS = 256;

class PatchedAil {
public:
  PatchedAil();
  ~PatchedAil();

private:
  HMODULE Module;

  // Original functions to replace
  static void(CALLBACK *OriginalWaveOutProc)(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                             DWORD_PTR dwParam2);
  static void *(WINAPI *OriginalAilFileRead)(char *filename, void *dest);
  static void(WINAPI *OriginalAilMemFreeLock)(void *ptr);

  // Globals
  static volatile WAVEHDR **pLastFinishedWaveHdr;
  static volatile WaveHdrUser **pLastFinishedWaveHdrUser;
  static volatile unsigned int *pWaveOutProcGlobalThing;

  // Hack to help mitigate the memory leaks while also preventing double frees
  static size_t AILAllocatedBlocks[MAX_AIL_ALLOCATED_BLOCKS];

  // Replacement functions
  static void CALLBACK WaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                   DWORD_PTR dwParam2);
  static void *WINAPI AilFileRead(char *filename, void *dest);
  static void WINAPI AilMemFreeLock(void *ptr);
};
