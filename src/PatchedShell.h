// Replacement functions for the game's shell (main menu and such), contained in MW2SHELL.DLL

#pragma once

#include "framework.h"
#include "PatchedAil.h"
#include "AudioSubsystem.h"

typedef int(__stdcall *ShellMainProc)(HMODULE module, int, const char *, int, HWND);

typedef void(__cdecl LoadMechVariantListFunc)(char *mechType);
typedef int(__stdcall CallsBitBlitFunc)(void);
typedef void(__cdecl ShellDebugLogFunc)(const char *format, ...);
typedef int(__fastcall LoadFileFromPrjFunc)(void *_this, void *_unused_thiscall, char *filename, int);

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
  static LoadMechVariantListFunc *OriginalLoadMechVariantList;
  static CallsBitBlitFunc *OriginalCallsBitBlit;
  static ShellDebugLogFunc *OriginalShellDebugLog;

  static AudioSubsystemConstructorFunc *OriginalAudioSubsystemConstructor;
  static AudioSubsystemDestructorFunc *OriginalAudioSubsystemDestructor;
  static GetDigitalDriverFunc *OriginalGetDigitalDriver;
  static CloseDigitalDriverFunc *OriginalCloseDigitalDriver;
  static ApplyMidiVolumeFunc *OriginalApplyMidiVolume;
  static GetActiveSequenceCountFunc *OriginalGetActiveSequenceCount;

  static AudioSampleConstructorFunc *OriginalAudioSampleConstructor;
  static AudioSampleDestructorFunc *OriginalAudioSampleDestructor;
  static StartFunc *OriginalStart;
  static GetIsPlayingFunc *OriginalGetIsPlaying;
  static SetFadeFunc *OriginalSetFade;
  static DoFadeFunc *OriginalDoFade;
  static EnableLoopFunc *OriginalEnableLoop;
  static SetLoopCountFunc *OriginalSetLoopCount;
  static SetVolumeFunc *OriginalSetVolume;

  static MidiSequenceConstructorFunc *OriginalMidiSequenceConstructor;
  static MidiSequenceDestructorFunc *OriginalMidiSequenceDestructor;
  static MidiSequenceStartFunc *OriginalMidiSequenceStart;
  static MidiSequenceStopFunc *OriginalMidiSequenceStop;
  static MidiSequenceApplyCurrentVolumeFunc *OriginalMidiSequenceApplyCurrentVolume;
  static MidiSequenceSetVolumeFunc *OriginalMidiSequenceSetVolume;
  static MidiSequenceSetLoopCountFunc *OriginalMidiSequenceSetLoopCount;
  static MidiSequenceGetGlobalActiveSequenceCountFunc *OriginalMidiSequenceGetGlobalActiveSequenceCount;

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
  static LoadFileFromPrjFunc *LoadFileFromPrj;

  // Replacement functions
  static void __cdecl LoadMechVariantList(char *mechType);
  static int __stdcall CallsBitBlit();
};
