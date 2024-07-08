#pragma once
#include "framework.h"
#include "soloud.h"
#include "soloud_wav.h"

class MidiSequence;

class AudioSubsystem {
public:
  AudioSubsystem();
  ~AudioSubsystem();
  SoLoud::Soloud *GetDigitalDriver();
  void CloseDigitalDriver();
  void ApplyMidiVolume();
  uint32_t GetActiveSequenceCount();

private:
  bool digitalInitialized;
  SoLoud::Soloud *soloud;
  MidiSequence *currentMidiSequence;
};

typedef struct {
  AudioSubsystem *proxy;
} AudioSubsystemProxy;

class AudioSample {
public:
  AudioSample(AudioSubsystem *audioSubsystem, unsigned char *audioData, int32_t audioDataSize);
  ~AudioSample();
  void Start();
  uint32_t GetIsPlaying();
  void SetFade(int32_t rate, int32_t max, int32_t startVol, int32_t endVol);
  void DoFade();
  void EnableLoop();
  void SetLoopCount(int32_t loopCount);
  void SetVolume(int32_t volume);

private:
  AudioSubsystem *subsystem;
  SoLoud::Wav *wave;
  SoLoud::handle playHandle;
  /// Volume in range 0-127
  int32_t volume;

  // Fade

  int32_t initialFadeRate;
  int32_t fadeRate;
  int32_t maxFade;
  int32_t startVolume;
  int32_t endVolume;
};

typedef struct {
  AudioSample *proxy;
} AudioSampleProxy;

class MidiSequence {
public:
  MidiSequence(AudioSubsystem *audioSubsystem, unsigned char *midiData, int32_t midiDataSize);
  ~MidiSequence();
  void Start();
  void Stop();
  void ApplyCurrentVolume();
  void SetVolume(int32_t volume);
  void SetLoopCount(int32_t loopCount);
  uint32_t GetGlobalActiveSequenceCount();

private:
  AudioSubsystem *subsystem;
  unsigned char *data;
  int32_t dataSize;
};

typedef struct {
  MidiSequence *proxy;
} MidiSequenceProxy;

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

typedef AudioSampleProxy *(__fastcall AudioSampleConstructorFunc)(AudioSampleProxy *_this, void *unused,
                                                                  AudioSubsystemProxy *audioSubsystemProxy,
                                                                  void *audioData, int32_t audioDataSize);
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

typedef MidiSequenceProxy *(__fastcall MidiSequenceConstructorFunc)(MidiSequenceProxy *_this, void *unused,
                                                                    AudioSubsystemProxy *audioSubsystemProxy,
                                                                    void *midiData, int32_t midiDataSize);
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
