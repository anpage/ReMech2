#include "PatchedSim.h"
#include "Remech2.h"

GameTickTimerCallbackFunc *PatchedSim::OriginalGameTickTimerCallback;
IntegerOverflowHappensHereFunc *PatchedSim::OriginalIntegerOverflowHappensHere;
SimDebugLogFunc *PatchedSim::OriginalSimDebugLog;
SetGameResolutionFunc *PatchedSim::OriginalSetGameResolution;
BlitFunc *PatchedSim::OriginalBlit;
SomeAspectRatioCalculationFunc *PatchedSim::OriginalSomeAspectRatioCalculation;
InitCdAudioFunc *PatchedSim::OriginalInitCdAudio;
GetGameCdNumberFunc *PatchedSim::OriginalGetGameCdNumber;
GetCdAudioAuxDeviceFunc *PatchedSim::OriginalGetCdAudioAuxDevice;
CloseCdAudioFunc *PatchedSim::OriginalCloseCdAudio;
PlayCdAudioFunc *PatchedSim::OriginalPlayCdAudio;
StartCdAudioFunc *PatchedSim::OriginalStartCdAudio;
GetCdStatusFunc *PatchedSim::OriginalGetCdStatus;
GetCdAudioTracksFunc *PatchedSim::OriginalGetCdAudioTracks;
GetCdAudioPositionFunc *PatchedSim::OriginalGetCdAudioPosition;
SetCdAudioVolumeFunc *PatchedSim::OriginalSetCdAudioVolume;
DeInitCdAudioFunc *PatchedSim::OriginalDeInitCdAudio;

volatile DWORD *PatchedSim::pTicksCheck;
volatile DWORD *PatchedSim::pTicks1;
volatile DWORD *PatchedSim::pTicks2;

volatile DWORD *PatchedSim::pGameWindowWidth;
volatile DWORD *PatchedSim::pGameWindowHeight;

volatile BOOL *PatchedSim::pBlitGlobal1;
volatile BOOL *PatchedSim::pWindowActive;
volatile DrawMode **PatchedSim::pCurrentDrawMode;
volatile WeirdRectStruct *PatchedSim::pStretchBlitSourceRect;
volatile WeirdRectStruct *PatchedSim::pStretchBlitOtherSourceRect;
volatile uint32_t *PatchedSim::pBlitGlobal2;
volatile uint32_t *PatchedSim::pBlitGlobal3;

volatile int32_t *PatchedSim::pWidthScale;
volatile int32_t **PatchedSim::pSomePointer;

volatile int32_t *PatchedSim::pCdAudioDevice;
volatile int32_t *PatchedSim::pCdAudioAuxDevice;

volatile uint32_t *PatchedSim::pCdAudioGlobal1;
volatile uint32_t *PatchedSim::pCdAudioGlobal2;
volatile uint32_t *PatchedSim::pCdAudioInitialized;
volatile AudioCdStatus *PatchedSim::pAudioCdStatus;
volatile CdAudioTracks *PatchedSim::pCdAudioTrackData;
volatile CdAudioPosition *PatchedSim::pPausedCdAudioPosition;
volatile int32_t *PatchedSim::pCdAudioVolume;

PatchedSim::PatchedSim() {
  if (Module != NULL) {
    DebugLog("ERROR: Multiple PatchedSim objects created?");
    exit(1);
  }

  Module = LoadLibraryA("MW2.DLL");
  if (Module == NULL) {
    exit(1);
  }

  MainProc = (SimMainProc)GetProcAddress(Module, "SimMain");
  if (MainProc == NULL) {
    exit(1);
  }

  SimWindowProc = (WNDPROC)GetProcAddress(Module, "SimWindowProc");
  if (SimWindowProc == NULL) {
    exit(1);
  }

  size_t baseAddress = (size_t)Module;

  // Original functions
  OriginalGameTickTimerCallback = (GameTickTimerCallbackFunc *)(baseAddress + 0x00067ed8);
  OriginalIntegerOverflowHappensHere = (IntegerOverflowHappensHereFunc *)(baseAddress + 0x000035a0);
  OriginalSimDebugLog = (SimDebugLogFunc *)(baseAddress + 0x00050958);
  OriginalSetGameResolution = (SetGameResolutionFunc *)(baseAddress + 0x00067e23);
  OriginalBlit = (BlitFunc *)(baseAddress + 0x00012e15);
  OriginalSomeAspectRatioCalculation = (SomeAspectRatioCalculationFunc *)(baseAddress + 0x0005d410);
  OriginalInitCdAudio = (InitCdAudioFunc *)(baseAddress + 0x0005a8b5);
  OriginalGetGameCdNumber = (GetGameCdNumberFunc *)(baseAddress + 0x00002df5);
  OriginalGetCdAudioAuxDevice = (GetCdAudioAuxDeviceFunc *)(baseAddress + 0x0005a7a0);
  OriginalCloseCdAudio = (CloseCdAudioFunc *)(baseAddress + 0x0005aa0a);
  OriginalPlayCdAudio = (PlayCdAudioFunc *)(baseAddress + 0x0005aabe);
  OriginalStartCdAudio = (StartCdAudioFunc *)(baseAddress + 0x0005ab0b);
  OriginalGetCdStatus = (GetCdStatusFunc *)(baseAddress + 0x0005ac33);
  OriginalGetCdAudioTracks = (GetCdAudioTracksFunc *)(baseAddress + 0x0005b49e);
  OriginalGetCdAudioPosition = (GetCdAudioPositionFunc *)(baseAddress + 0x0005b61d);
  OriginalSetCdAudioVolume = (SetCdAudioVolumeFunc *)(baseAddress + 0x0005b734);
  OriginalDeInitCdAudio = (DeInitCdAudioFunc *)(baseAddress + 0x0005abff);

  // Globals
  pTicksCheck = (DWORD *)(baseAddress + 0x000ad008);
  pTicks1 = (DWORD *)(baseAddress + 0x000ad20c);
  pTicks2 = (DWORD *)(baseAddress + 0x000ad210);
  pGameWindowWidth = (DWORD *)(baseAddress + 0x000acb6c);
  pGameWindowHeight = (DWORD *)(baseAddress + 0x000acb70);
  pBlitGlobal1 = (BOOL *)(baseAddress + 0x00176ebc);
  pWindowActive = (BOOL *)(baseAddress + 0x000acb74);
  pCurrentDrawMode = (volatile DrawMode **)(baseAddress + 0x000b1774);
  pStretchBlitSourceRect = (WeirdRectStruct *)(baseAddress + 0x00176ed0);
  pStretchBlitOtherSourceRect = (WeirdRectStruct *)(baseAddress + 0x000bdff8);
  pBlitGlobal2 = (uint32_t *)(baseAddress + 0x000a5f18);
  pBlitGlobal3 = (uint32_t *)(baseAddress + 0x000a5a24);
  pWidthScale = (int32_t *)(baseAddress + 0x000e9610);
  pSomePointer = (volatile int **)(baseAddress + 0x000a6cc0);
  pCdAudioDevice = (int32_t *)(baseAddress + 0x000aa278);
  pCdAudioAuxDevice = (int32_t *)(baseAddress + 0x000aa27c);
  pCdAudioGlobal1 = (uint32_t *)(baseAddress + 0x000beca8);
  pCdAudioGlobal2 = (uint32_t *)(baseAddress + 0x000becac);
  pCdAudioInitialized = (uint32_t *)(baseAddress + 0x000aa28c);
  pAudioCdStatus = (AudioCdStatus *)(baseAddress + 0x000becc0);
  pCdAudioTrackData = (CdAudioTracks *)(baseAddress + 0x000aa280);
  pPausedCdAudioPosition = (CdAudioPosition *)(baseAddress + 0x000becb0);
  pCdAudioVolume = (int32_t *)(baseAddress + 0x000a14a4);

  DetourTransactionBegin();
  DetourAttach((PVOID *)(&OriginalGameTickTimerCallback), GameTickTimerCallback);
  DetourAttach((PVOID *)(&OriginalIntegerOverflowHappensHere), IntegerOverflowHappensHere);
  DetourAttach((PVOID *)(&OriginalSimDebugLog), DebugLog);
  DetourAttach((PVOID *)(&OriginalSetGameResolution), SetGameResolution);
  DetourAttach((PVOID *)(&OriginalBlit), Blit);
  DetourAttach((PVOID *)(&OriginalSomeAspectRatioCalculation), SomeAspectRatioCalculation);
  DetourAttach((PVOID *)(&OriginalInitCdAudio), InitCdAudio);
  DetourAttach((PVOID *)(&OriginalPlayCdAudio), PlayCdAudio);
  DetourAttach((PVOID *)(&OriginalGetCdStatus), GetCdStatus);
  DetourAttach((PVOID *)(&OriginalStartCdAudio), StartCdAudio);
  DetourAttach((PVOID *)(&TrueRegCreateKeyExA), FakeRegCreateKeyExA);
  DetourAttach((PVOID *)(&TrueRegOpenKeyExA), FakeRegOpenKeyExA);
  DetourAttach((PVOID *)(&TrueTimeSetEvent), FakeTimeSetEvent);
  DetourTransactionCommit();

  Ail = new PatchedAil();
}

PatchedSim::~PatchedSim() {
  DetourTransactionBegin();
  DetourDetach((PVOID *)(&OriginalGameTickTimerCallback), GameTickTimerCallback);
  DetourDetach((PVOID *)(&OriginalIntegerOverflowHappensHere), IntegerOverflowHappensHere);
  DetourDetach((PVOID *)(&OriginalSimDebugLog), DebugLog);
  DetourDetach((PVOID *)(&OriginalSetGameResolution), SetGameResolution);
  DetourDetach((PVOID *)(&OriginalBlit), Blit);
  DetourDetach((PVOID *)(&OriginalSomeAspectRatioCalculation), SomeAspectRatioCalculation);
  DetourDetach((PVOID *)(&OriginalInitCdAudio), InitCdAudio);
  DetourDetach((PVOID *)(&OriginalPlayCdAudio), PlayCdAudio);
  DetourDetach((PVOID *)(&OriginalGetCdStatus), GetCdStatus);
  DetourDetach((PVOID *)(&OriginalStartCdAudio), StartCdAudio);
  DetourDetach((PVOID *)(&TrueRegCreateKeyExA), FakeRegCreateKeyExA);
  DetourDetach((PVOID *)(&TrueRegOpenKeyExA), FakeRegOpenKeyExA);
  DetourDetach((PVOID *)(&TrueTimeSetEvent), FakeTimeSetEvent);
  DetourTransactionCommit();
  FreeLibrary(Module);
  Module = NULL;
  delete Ail;
}

int PatchedSim::SimMain(LPSTR cmdLine, void **unknown, BOOL isNetGame, HWND window) {
  DebugLog("Entering sim...\n");

  int result = MainProc(Module, 0, cmdLine, unknown, isNetGame, window);
  DebugLog("Returned from Sim, retval=%d.\n", result);
  return result;
}

// The original function was corrupting the stack with my custom PatchedAil::TimeProc.
// Replacing it with this freshly recompiled copy fixed the problem (for now?)
void __stdcall PatchedSim::GameTickTimerCallback(DWORD unused) {
  if ((*pTicksCheck & 0x200) == 0) {
    (*pTicks1)++;
  }
  if ((*pTicksCheck & 0x100) == 0) {
    (*pTicks2)++;
  }
}

// This function is used all over the game to perform ((a * b) / c).
// It sometimes overflows and sometimes divides by zero, especially when the FPS is too high.
// TODO: Fix that, probably.
int32_t __cdecl PatchedSim::IntegerOverflowHappensHere(int32_t param1, int32_t param2, int32_t param3) {
  return (int32_t)(((int64_t)param1 * (int64_t)param2) / (int64_t)param3);
}

// TODO: Allow setting arbitrary resolutions
void __cdecl PatchedSim::SetGameResolution(char *driverName) {
  *pGameWindowWidth = 320;
  *pGameWindowHeight = 200;

  int result = _strcmpi(driverName, "VESA480.DLL");
  if (result == 0) {
    *pGameWindowWidth = 640;
    *pGameWindowHeight = 480;
    return;
  }

  result = _strcmpi(driverName, "VESA768.DLL");
  if (result == 0) {
    *pGameWindowWidth = 1024;
    *pGameWindowHeight = 768;
    return;
  }
}

static DWORD nexttick = timeGetTime();
static void LimitFramerate(int rate) {
  nexttick += rate;
  while (timeGetTime() < nexttick)
    ;
}

// Hook the game's Blit() function to add a framerate limit
void __stdcall PatchedSim::Blit() {
  LimitFramerate(1000 / 45);

  if (*pBlitGlobal1 == 0) {
    if (*pWindowActive != 0) {
      (*pCurrentDrawMode)->blitFlipFunc();
    }
  } else {
    (*pCurrentDrawMode)
        ->stretchBlitFunc((*pStretchBlitSourceRect).x1 + 1, (*pStretchBlitSourceRect).y2 + 1,
                          (*pStretchBlitSourceRect).x2, (*pStretchBlitSourceRect).y1);
    WeirdRectStruct *pWVar2 = (WeirdRectStruct *)pStretchBlitOtherSourceRect;
    WeirdRectStruct *pWVar3 = (WeirdRectStruct *)pStretchBlitSourceRect;

    memcpy((void *)pStretchBlitSourceRect, (const void *)pStretchBlitOtherSourceRect, sizeof(WeirdRectStruct));

    *pBlitGlobal2 = *pBlitGlobal3;
    *pBlitGlobal1 = 0;
  }
}

// Always return 0.999 (in fixed-point format) to disable stretching in widescreen.
// I think the game used this calculation to stretch out 320x200 so that it could
// be squished back to 4:3 by old monitors.
void __cdecl PatchedSim::SomeAspectRatioCalculation(SomeAspectRatioStruct *someStruct) {
  *pWidthScale = 0xFFFF;
  (*pSomePointer)[0x11] = *pWidthScale;
}

// Fixed CD audio by not trying to use any specific drive, just the first one with CD audio
uint32_t __stdcall PatchedSim::InitCdAudio() {
  MCI_OPEN_PARMSA mciOpenParms{};
  mciOpenParms.lpstrDeviceType = "cdaudio";
  MCIERROR mciOpenError = mciSendCommandA(0, MCI_OPEN, MCI_OPEN_TYPE, (DWORD_PTR)&mciOpenParms);
  if (mciOpenError != 0) {
    return 1;
  }

  *pCdAudioDevice = mciOpenParms.wDeviceID;

  MCI_SET_PARMS mciSetParms{};
  mciSetParms.dwTimeFormat = MCI_FORMAT_TMSF;
  MCIERROR mciSetError = mciSendCommandA(mciOpenParms.wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSetParms);
  if (mciSetError != 0) {
    OriginalCloseCdAudio();
    return 1;
  }

  *pCdAudioAuxDevice = OriginalGetCdAudioAuxDevice();
  return 0;
}

void __cdecl PatchedSim::PlayCdAudio(DWORD from, DWORD to) {
  DWORD_PTR dwFlags = MCI_FROM;

  MCI_PLAY_PARMS mciPlayParms{};
  mciPlayParms.dwFrom = from;
  if (to != 0) {
    dwFlags = 0xC;
    mciPlayParms.dwTo = to;
  }
  MCIERROR mciError = mciSendCommandA(*pCdAudioDevice, MCI_PLAY, dwFlags, (DWORD_PTR)&mciPlayParms);
  MCI_MODE_NOT_READY;
}

int32_t PatchedSim::StartCdAudio() {
  *pCdAudioGlobal1 = 0;
  *pCdAudioGlobal2 = 0;

  int initCdAudioResult = InitCdAudio();
  if (initCdAudioResult != 0) {
    return 0;
  }

  *pCdAudioInitialized = 1;

  *pAudioCdStatus = GetCdStatus();
  switch (*pAudioCdStatus) {
  case OPEN:
    *pCdAudioInitialized = 0;
    return 0;
  case STOPPED:
    OriginalGetCdAudioTracks((CdAudioTracks *)pCdAudioTrackData);
    break;
  case PLAYING:
    OriginalGetCdAudioTracks((CdAudioTracks *)pCdAudioTrackData);
    break;
  case PAUSED:
    OriginalGetCdAudioTracks((CdAudioTracks *)pCdAudioTrackData);
    OriginalGetCdAudioPosition((CdAudioPosition *)pPausedCdAudioPosition);
  }

  OriginalSetCdAudioVolume(*pCdAudioVolume);
  return 1;
}

AudioCdStatus __cdecl PatchedSim::GetCdStatus() {
  if (*pCdAudioInitialized == 0) {
    return CD_ERROR;
  }

  MCI_STATUS_PARMS mciStatusParms{};
  mciStatusParms.dwItem = MCI_STATUS_MODE;
  MCIERROR mciError = mciSendCommandA(*pCdAudioDevice, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms);
  if (mciError != 0) {
    OriginalDeInitCdAudio();
    return CD_ERROR;
  }

  switch (mciStatusParms.dwReturn) {
    // Some CD emulation software reports MCI_MODE_OPEN when stopped
  case MCI_MODE_OPEN:
  case MCI_MODE_STOP:
    return STOPPED;
  case MCI_MODE_PLAY:
    return PLAYING;
  case MCI_MODE_PAUSE:
    return PAUSED;
  }

  return CD_ERROR;
}