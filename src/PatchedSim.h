#pragma once

// Replacement functions for the game's sim (in-game missions), contained in MW2.DLL

#pragma once

#include "framework.h"
#include "PatchedAil.h"

struct SomeDDrawStruct {
  LPRECT rect;
  int32_t unknown1;
  int32_t unknown2;
  BITMAPINFO **bitmapInfo;
  uint32_t unknown3;
};

struct WeirdRectStruct {
  SomeDDrawStruct *someDDRawStruct;
  int32_t x1;
  int32_t y2;
  int32_t x2;
  int32_t y1;
};

typedef int32_t(__cdecl *DrawModeInitFunc)(SomeDDrawStruct *, int32_t, int32_t);
typedef int32_t(__cdecl *DrawModeDeInitFunc)(void);
typedef int32_t(__cdecl *DrawModeBlitFlipFunc)(void);
typedef int32_t(__cdecl *DrawModeBlitRectFunc)(int32_t, int32_t, int32_t, int32_t);
typedef int32_t(__cdecl *DrawModeStretchBlitFunc)(uint32_t, uint32_t, int32_t, int32_t);

struct DrawMode {
  uint32_t index;
  int32_t someIndexToRelatedStruct;
  int32_t initialized;
  uint32_t unknown1;
  DrawModeInitFunc initFunc;
  DrawModeDeInitFunc deInitFunc;
  DrawModeBlitFlipFunc blitFlipFunc;
  DrawModeBlitRectFunc blitRectFunc;
  DrawModeStretchBlitFunc stretchBlitFunc;
  uint32_t unknown2;
};

struct SomeAspectRatioStruct {
  int32_t width;
  int32_t height;
  int32_t unknown1;
  int32_t unknown2;
  int32_t unknown3;
  int32_t unknown4;
};

enum AudioCdStatus { UNKNOWN, OPEN, STOPPED, PLAYING, PAUSED, CD_ERROR };

struct CdAudioTracks {
  uint32_t firstTrack;
  uint32_t numberOfTracks;
  uint32_t *trackPositions;
};

struct CdAudioPosition {
  uint32_t track;
  uint32_t minute;
  uint32_t second;
  uint32_t frame;
};

typedef int(__stdcall *SimMainProc)(HMODULE module, unsigned int, LPSTR cmdLine, void **, BOOL isNetGame, HWND window);

typedef void(__stdcall GameTickTimerCallbackFunc)(DWORD);
typedef int32_t(__cdecl IntegerOverflowHappensHereFunc)(int32_t, int32_t, int32_t);
typedef void(__cdecl SimDebugLogFunc)(const char *format, ...);
typedef void(__cdecl SetGameResolutionFunc)(char *driverName);
typedef void(__stdcall BlitFunc)(void);
typedef void(__cdecl SomeAspectRatioCalculationFunc)(SomeAspectRatioStruct *someStruct);
typedef uint32_t(__stdcall InitCdAudioFunc)(void);
typedef int32_t(__stdcall GetGameCdNumberFunc)(void);
typedef int32_t(__stdcall GetCdAudioAuxDeviceFunc)(void);
typedef uint32_t(__stdcall CloseCdAudioFunc)(void);
typedef void(__cdecl PlayCdAudioFunc)(DWORD from, DWORD to);
typedef int32_t(__stdcall StartCdAudioFunc)(void);
typedef AudioCdStatus(__cdecl GetCdStatusFunc)(void);
typedef int32_t(__cdecl GetCdAudioTracksFunc)(CdAudioTracks *tracks);
typedef void(__cdecl GetCdAudioPositionFunc)(CdAudioPosition *position);
typedef int32_t(__cdecl SetCdAudioVolumeFunc)(int32_t volume);
typedef void(__stdcall DeInitCdAudioFunc)(void);
typedef void(__stdcall HandleMessagesFunc)(void);

class PatchedSim {
public:
  PatchedSim();
  ~PatchedSim();
  int SimMain(LPSTR cmdLine, void **unknown, BOOL isNetGame, HWND window);

  // Original functions to replace
  static GameTickTimerCallbackFunc *OriginalGameTickTimerCallback;
  static IntegerOverflowHappensHereFunc *OriginalIntegerOverflowHappensHere;
  static SimDebugLogFunc *OriginalSimDebugLog;
  static SetGameResolutionFunc *OriginalSetGameResolution;
  static BlitFunc *OriginalBlit;
  static SomeAspectRatioCalculationFunc *OriginalSomeAspectRatioCalculation;
  static InitCdAudioFunc *OriginalInitCdAudio;
  static GetGameCdNumberFunc *OriginalGetGameCdNumber;
  static GetCdAudioAuxDeviceFunc *OriginalGetCdAudioAuxDevice;
  static CloseCdAudioFunc *OriginalCloseCdAudio;
  static PlayCdAudioFunc *OriginalPlayCdAudio;
  static StartCdAudioFunc *OriginalStartCdAudio;
  static GetCdStatusFunc *OriginalGetCdStatus;
  static GetCdAudioTracksFunc *OriginalGetCdAudioTracks;
  static GetCdAudioPositionFunc *OriginalGetCdAudioPosition;
  static SetCdAudioVolumeFunc *OriginalSetCdAudioVolume;
  static DeInitCdAudioFunc *OriginalDeInitCdAudio;
  static HandleMessagesFunc *OriginalHandleMessages;

  // Globals
  static volatile DWORD *pTicksCheck;
  static volatile DWORD *pTicks1;
  static volatile DWORD *pTicks2;
  static volatile DWORD *pGameWindowWidth;
  static volatile DWORD *pGameWindowHeight;
  static volatile BOOL *pBlitGlobal1;
  static volatile BOOL *pWindowActive;
  static volatile DrawMode **pCurrentDrawMode;
  static volatile WeirdRectStruct *pStretchBlitSourceRect;
  static volatile WeirdRectStruct *pStretchBlitOtherSourceRect;
  static volatile uint32_t *pBlitGlobal2;
  static volatile uint32_t *pBlitGlobal3;
  static volatile int32_t *pWidthScale;
  static volatile int32_t **pSomePointer;
  static volatile int32_t *pCdAudioDevice;
  static volatile int32_t *pCdAudioAuxDevice;
  static volatile uint32_t *pCdAudioGlobal1;
  static volatile uint32_t *pCdAudioGlobal2;
  static volatile uint32_t *pCdAudioInitialized;
  static volatile AudioCdStatus *pAudioCdStatus;
  static volatile CdAudioTracks *pCdAudioTrackData;
  static volatile CdAudioPosition *pPausedCdAudioPosition;
  static volatile int32_t *pCdAudioVolume;
  static volatile BOOL *pMessagesHandled;

  static GameTickTimerCallbackFunc GameTickTimerCallback;
  static IntegerOverflowHappensHereFunc IntegerOverflowHappensHere;
  static SetGameResolutionFunc SetGameResolution;
  static BlitFunc Blit;
  static SomeAspectRatioCalculationFunc SomeAspectRatioCalculation;
  static InitCdAudioFunc InitCdAudio;
  static PlayCdAudioFunc PlayCdAudio;
  static StartCdAudioFunc StartCdAudio;
  static GetCdStatusFunc GetCdStatus;
  static HandleMessagesFunc HandleMessages;

private:
  HMODULE Module;
  SimMainProc MainProc;
  PatchedAil *Ail;
};
