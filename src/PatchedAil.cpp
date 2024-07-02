#include "PatchedAil.h"
#include "ReMech2.h"
#include <algorithm>

void(CALLBACK *PatchedAil::OriginalWaveOutProc)(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                                DWORD_PTR dwParam2);
void *(WINAPI *PatchedAil::OriginalAilFileRead)(char *filename, void *dest);
void(WINAPI *PatchedAil::OriginalAilMemFreeLock)(void *ptr);
LPTIMECALLBACK PatchedAil::OriginalTimeProc;

volatile WAVEHDR **PatchedAil::pLastFinishedWaveHdr;
volatile WaveHdrUser **PatchedAil::pLastFinishedWaveHdrUser;
volatile unsigned int *PatchedAil::pWaveOutProcGlobalThing;

volatile DWORD *PatchedAil::pPeriod;
volatile DWORD *PatchedAil::pCounter;
volatile SomeTimerStruct **PatchedAil::pTimers;
volatile DWORD *PatchedAil::pGlobal3;
volatile BOOL *PatchedAil::pTimeProcLocked;
volatile DWORD *PatchedAil::pGlobal5;
volatile DWORD *PatchedAil::pNumTimers;

size_t PatchedAil::AILAllocatedBlocks[MAX_AIL_ALLOCATED_BLOCKS];

PatchedAil::PatchedAil() {
  if (Module != NULL) {
    DebugLog("ERROR: Multiple PatchedAil objects created?");
    exit(1);
  }

  Module = GetModuleHandleA("WAIL32.DLL");
  if (Module == NULL) {
    exit(1);
  }

  size_t baseAddress = (size_t)Module;

  OriginalWaveOutProc = (void(CALLBACK *)(HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR))(baseAddress + 0x00008e6d);
  OriginalAilFileRead = (void *(WINAPI *)(char *, void *))(baseAddress + 0x0000845f);
  OriginalAilMemFreeLock = (void(WINAPI *)(void *))(baseAddress + 0x00001f14);
  OriginalTimeProc = (LPTIMECALLBACK)(baseAddress + 0x000011c6);

  pLastFinishedWaveHdr = (volatile WAVEHDR **)(baseAddress + 0x0001ba10);
  pLastFinishedWaveHdrUser = (volatile WaveHdrUser **)(baseAddress + 0x0001ba0c);
  pWaveOutProcGlobalThing = (unsigned int *)(baseAddress + 0x0001ba04);

  pPeriod = (DWORD *)(baseAddress + 0x0001b7fc);
  pCounter = (DWORD *)(baseAddress + 0x0001b810);
  pTimers = (volatile SomeTimerStruct **)(baseAddress + 0x0001b7f8);
  pGlobal3 = (DWORD *)(baseAddress + 0x0001b804);
  pTimeProcLocked = (BOOL *)(baseAddress + 0x00019030);
  pGlobal5 = (DWORD *)(baseAddress + 0x0001c59c);
  pNumTimers = (DWORD *)(baseAddress + 0x0001b800);

  // Clear allocated blocks
  std::fill(std::begin(AILAllocatedBlocks), std::end(AILAllocatedBlocks), 0);

  DetourTransactionBegin();
  DetourAttach((PVOID *)(&OriginalWaveOutProc), WaveOutProc);
  DetourAttach((PVOID *)(&OriginalAilFileRead), AilFileRead);
  DetourAttach((PVOID *)(&OriginalAilMemFreeLock), AilMemFreeLock);
  DetourAttach((PVOID *)(&OriginalTimeProc), TimeProc);
  DetourTransactionCommit();
}

PatchedAil::~PatchedAil() {
  DetourTransactionBegin();
  DetourDetach((PVOID *)(&OriginalWaveOutProc), WaveOutProc);
  DetourDetach((PVOID *)(&OriginalAilFileRead), AilFileRead);
  DetourDetach((PVOID *)(&OriginalAilMemFreeLock), AilMemFreeLock);
  DetourDetach((PVOID *)(&OriginalTimeProc), TimeProc);
  DetourTransactionCommit();
  FreeLibrary(Module);
  Module = NULL;
}

// Replacement for AIL's waveOutOpen callback that doesn't try to suspend the main thread
void CALLBACK PatchedAil::WaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                      DWORD_PTR dwParam2) {
  if (uMsg == WOM_DONE) {
    WAVEHDR *waveHdr = (WAVEHDR *)dwParam1;
    *pLastFinishedWaveHdr = waveHdr;
    if (waveHdr->dwUser != 0) {
      *pLastFinishedWaveHdrUser = *(WaveHdrUser **)waveHdr->dwUser;
      if ((*pLastFinishedWaveHdrUser)->unknown6 != 0) {
        *pWaveOutProcGlobalThing = (*pLastFinishedWaveHdrUser)->unknown4;
        *(WAVEHDR **)((*pLastFinishedWaveHdrUser)->unknown3 + *pWaveOutProcGlobalThing * 4) = waveHdr;
        *pWaveOutProcGlobalThing = (*pWaveOutProcGlobalThing + 1) % (*pLastFinishedWaveHdrUser)->unknown2;
        (*pLastFinishedWaveHdrUser)->unknown4 = *pWaveOutProcGlobalThing;
      }
    }
  }
}

void *WINAPI PatchedAil::AilFileRead(char *filename, void *dest) {
  void *result = OriginalAilFileRead(filename, dest);
  if (dest == NULL) {
    for (size_t i = 0; i < MAX_AIL_ALLOCATED_BLOCKS; i++) {
      if (AILAllocatedBlocks[i] == NULL) {
        AILAllocatedBlocks[i] = (size_t)result;
        break;
      }
    }
  }
  return result;
}

// Only try to free blocks that we know haven't been freed yet
void WINAPI PatchedAil::AilMemFreeLock(void *ptr) {
  for (size_t i = 0; i < MAX_AIL_ALLOCATED_BLOCKS; i++) {
    if (AILAllocatedBlocks[i] == (size_t)ptr) {
      OriginalAilMemFreeLock(ptr);
      AILAllocatedBlocks[i] = 0;
      return;
    }
  }
  return;
}

// Passed to timeSetEvent in AIL to handle timers
// This also had calls to SuspendThread that needed to be removed
void __stdcall PatchedAil::TimeProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
  SomeTimerStruct *timers = (SomeTimerStruct *)*pTimers;
  int NumTimers = *pNumTimers;

  (*pCounter)++;

  if (timers == NULL || *pGlobal3 > 0 || *pTimeProcLocked) {
    return;
  }

  (*pTimeProcLocked) = TRUE;
  (*pGlobal5)++;
  for (int i = 0; i < NumTimers; i++) {
    if (timers[i].probablyState == 2) {
      timers[i].accumulatedTime += (*pPeriod);
      if (timers[i].accumulatedTime >= timers[i].nextProcTime) {
        timers[i].accumulatedTime -= timers[i].nextProcTime;
        timers[i].callback(timers[i].unknown);
      }
    }
  }
  (*pGlobal5)--;
  (*pTimeProcLocked) = FALSE;
}
