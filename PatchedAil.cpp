#include "PatchedAil.h"
#include "ReMech2.h"
#include <algorithm>

void(CALLBACK *PatchedAil::OriginalWaveOutProc)(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                                DWORD_PTR dwParam2);
void *(WINAPI *PatchedAil::OriginalAilFileRead)(char *filename, void *dest);
void(WINAPI *PatchedAil::OriginalAilMemFreeLock)(void *ptr);

volatile WAVEHDR **PatchedAil::pLastFinishedWaveHdr;
volatile WaveHdrUser **PatchedAil::pLastFinishedWaveHdrUser;
volatile unsigned int *PatchedAil::pWaveOutProcGlobalThing;

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

  pLastFinishedWaveHdr = (volatile WAVEHDR **)(baseAddress + 0x0001ba10);
  pLastFinishedWaveHdrUser = (volatile WaveHdrUser **)(baseAddress + 0x0001ba0c);
  pWaveOutProcGlobalThing = (unsigned int *)(baseAddress + 0x0001ba04);

  // Clear allocated blocks
  std::fill(std::begin(AILAllocatedBlocks), std::end(AILAllocatedBlocks), 0);

  DetourTransactionBegin();
  DetourAttach((PVOID *)(&OriginalWaveOutProc), WaveOutProc);
  DetourAttach((PVOID *)(&OriginalAilFileRead), AilFileRead);
  DetourAttach((PVOID *)(&OriginalAilMemFreeLock), AilMemFreeLock);
  DetourTransactionCommit();
}

PatchedAil::~PatchedAil() {
  DetourTransactionBegin();
  DetourDetach((PVOID *)(&OriginalWaveOutProc), WaveOutProc);
  DetourDetach((PVOID *)(&OriginalAilFileRead), AilFileRead);
  DetourDetach((PVOID *)(&OriginalAilMemFreeLock), AilMemFreeLock);
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
      if ((*pLastFinishedWaveHdrUser)->field_4 != 0) {
        *pWaveOutProcGlobalThing = (*pLastFinishedWaveHdrUser)->field_3;
        *(WAVEHDR **)((*pLastFinishedWaveHdrUser)->field_2 + *pWaveOutProcGlobalThing * 4) = waveHdr;
        *pWaveOutProcGlobalThing = (*pWaveOutProcGlobalThing + 1) % (*pLastFinishedWaveHdrUser)->field_1;
        (*pLastFinishedWaveHdrUser)->field_3 = *pWaveOutProcGlobalThing;
      }
    }
  }
}

void *WINAPI PatchedAil::AilFileRead(char *filename, void *dest) {
  size_t filenameLen = strlen(filename) + 1;
  char *newFilename = (char *)malloc(filenameLen + 1);
  if (newFilename == NULL) {
    return NULL;
  }
  strcpy(newFilename, filename);
  if (filename[1] == ':') {
    strncpy(newFilename, filename, 2);
    newFilename[2] = '\\';
    strncpy(newFilename + 3, filename + 2, filenameLen - 2);
    newFilename[filenameLen] = '\0';
  }
  // HANDLE file = CreateFileA(newFilename, 0x80000000, 1, (LPSECURITY_ATTRIBUTES)0x0, 3, 0x8000080, (HANDLE)0x0);
  void *result = OriginalAilFileRead(newFilename, dest);
  if (dest == NULL) {
    for (size_t i = 0; i < MAX_AIL_ALLOCATED_BLOCKS; i++) {
      if (AILAllocatedBlocks[i] == NULL) {
        AILAllocatedBlocks[i] = (size_t)result;
        break;
      }
    }
  }
  free(newFilename);
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
