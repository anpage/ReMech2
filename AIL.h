// Hooks to work around issues with the old version of AIL.
// A temporary hack until the game can use a more modern audio library.

#pragma once

#include "framework.h"

// Structs
struct WaveHdrUser {
  char unknown_1[20];
  unsigned int field_1;
  int field_2;
  unsigned int field_3;
  char unknown_2[40];
  int field_4;
};

// Globals
extern volatile WAVEHDR **pLastFinishedWaveHdr;
extern volatile WaveHdrUser **pLastFinishedWaveHdrUser;
extern volatile unsigned int *pWaveOutProcGlobalThing;

constexpr auto MAX_AIL_ALLOCATED_BLOCKS = 256;
extern size_t AILAllocatedBlocks[MAX_AIL_ALLOCATED_BLOCKS];

// Original functions
extern void(CALLBACK *TrueWaveOutProc)(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                       DWORD_PTR dwParam2);
extern void *(WINAPI *TrueAilFileRead)(char *filename, void *dest);
extern void(WINAPI *TrueAilMemFreeLock)(void *ptr);

// Replacement functions
void CALLBACK FakeWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
void *WINAPI FakeAilFileRead(char *filename, void *dest);
void WINAPI FakeAilMemFreeLock(void *ptr);
