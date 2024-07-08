#pragma once
#include "framework.h"
#include "soloud.h"

class AudioSubsystem {
public:
  AudioSubsystem();
  ~AudioSubsystem();
  void *GetDigitalDriver();
  void CloseDigitalDriver();
  void ApplyMidiVolume();
  uint32_t GetActiveSequenceCount();
};

typedef struct {
  AudioSubsystem *proxy;
} AudioSubsystemProxy;

typedef AudioSubsystemProxy *(__fastcall AudioSubsystemConstructorFunc)(AudioSubsystemProxy *_this);
extern AudioSubsystemConstructorFunc AudioSubsystemConstructor;

typedef void(__fastcall AudioSubsystemDestructorFunc)(AudioSubsystemProxy *_this);
extern AudioSubsystemDestructorFunc AudioSubsystemDestructor;

typedef void *(__fastcall GetDigitalDriverFunc)(AudioSubsystemProxy *_this);
extern GetDigitalDriverFunc AudioSubsystemGetDigitalDriver;

typedef void(__fastcall CloseDigitalDriverFunc)(AudioSubsystemProxy *_this);
extern CloseDigitalDriverFunc AudioSubsystemCloseDigitalDriver;

typedef void(__fastcall ApplyMidiVolumeFunc)(AudioSubsystemProxy *_this);
extern ApplyMidiVolumeFunc AudioSubsystemApplyMidiVolume;

typedef uint32_t(__fastcall GetActiveSequenceCountFunc)(AudioSubsystemProxy *_this);
extern GetActiveSequenceCountFunc AudioSubsystemGetActiveSequenceCount;

class AudioSample {
public:
  AudioSample();
  ~AudioSample();
  void Start();
  uint32_t GetIsPlaying();
  void SetFade(int32_t rate, int32_t max, int32_t startVol, int32_t endVol);
  void DoFade();
  void EnableLoop();
  void SetLoopCount(int32_t loopCount);
  void SetVolume(int32_t volume);
};

typedef struct {
  AudioSample *proxy;
} AudioSampleProxy;

typedef AudioSampleProxy *(__fastcall AudioSampleConstructorFunc)(AudioSampleProxy *_this, void *unused,
                                                                  AudioSubsystem *audioSubsystem, void *audioData,
                                                                  int32_t audioDataSize);
extern AudioSampleConstructorFunc AudioSampleConstructor;

typedef void(__fastcall AudioSampleDestructorFunc)(AudioSampleProxy *_this);
extern AudioSampleDestructorFunc AudioSampleDestructor;

typedef void(__fastcall StartFunc)(AudioSampleProxy *_this);
extern StartFunc AudioSampleStart;

typedef uint32_t(__fastcall GetIsPlayingFunc)(AudioSampleProxy *_this);
extern GetIsPlayingFunc AudioSampleGetIsPlaying;

typedef void(__fastcall SetFadeFunc)(AudioSampleProxy *_this, void *unused, int32_t rate, int32_t max, int32_t startVol,
                                     int32_t endVol);
extern SetFadeFunc AudioSampleSetFade;

typedef void(__fastcall DoFadeFunc)(AudioSampleProxy *_this);
extern DoFadeFunc AudioSampleDoFade;

typedef void(__fastcall EnableLoopFunc)(AudioSampleProxy *_this);
extern EnableLoopFunc AudioSampleEnableLoop;

typedef void(__fastcall SetLoopCountFunc)(AudioSampleProxy *_this, void *unused, int32_t loopCount);
extern SetLoopCountFunc AudioSampleSetLoopCount;

typedef void(__fastcall SetVolumeFunc)(AudioSampleProxy *_this, void *unused, int32_t volume);
extern SetVolumeFunc AudioSampleSetVolume;

class MidiSequence {
public:
  MidiSequence(AudioSubsystem *audioSubsystem, void *audioData, int32_t audioDataSize);
  ~MidiSequence();
  void Start();
  void Stop();
  void ApplyCurrentVolume();
  void SetVolume(int32_t volume);
  void SetLoopCount(int32_t loopCount);
  uint32_t GetGlobalActiveSequenceCount();
};

typedef struct {
  MidiSequence *proxy;
} MidiSequenceProxy;

typedef MidiSequenceProxy *(__fastcall MidiSequenceConstructorFunc)(MidiSequenceProxy *_this, void *unused,
                                                                    AudioSubsystem *audioSubsystem, void *audioData,
                                                                    int32_t audioDataSize);
extern MidiSequenceConstructorFunc MidiSequenceConstructor;

typedef void(__fastcall MidiSequenceDestructorFunc)(MidiSequenceProxy *_this);
extern MidiSequenceDestructorFunc MidiSequenceDestructor;

typedef void(__fastcall MidiSequenceStartFunc)(MidiSequenceProxy *_this);
extern MidiSequenceStartFunc MidiSequenceStart;

typedef void(__fastcall MidiSequenceStopFunc)(MidiSequenceProxy *_this);
extern MidiSequenceStopFunc MidiSequenceStop;

typedef void(__fastcall MidiSequenceApplyCurrentVolumeFunc)(MidiSequenceProxy *_this);
extern MidiSequenceApplyCurrentVolumeFunc MidiSequenceApplyCurrentVolume;

typedef void(__fastcall MidiSequenceSetVolumeFunc)(MidiSequenceProxy *_this, void *unused, int32_t volume);
extern MidiSequenceSetVolumeFunc MidiSequenceSetVolume;

typedef void(__fastcall MidiSequenceSetLoopCountFunc)(MidiSequenceProxy *_this, void *unused, int32_t loopCount);
extern MidiSequenceSetLoopCountFunc MidiSequenceSetLoopCount;

typedef uint32_t(__fastcall MidiSequenceGetGlobalActiveSequenceCountFunc)(MidiSequenceProxy *_this);
extern MidiSequenceGetGlobalActiveSequenceCountFunc MidiSequenceGetGlobalActiveSequenceCount;
