#include "AudioSubsystem.h"

AudioSubsystem::AudioSubsystem() {}
AudioSubsystem::~AudioSubsystem() {}
void *AudioSubsystem::GetDigitalDriver() { return nullptr; }
void AudioSubsystem::CloseDigitalDriver() {}
void AudioSubsystem::ApplyMidiVolume() {}
uint32_t AudioSubsystem::GetActiveSequenceCount() { return 0; }

AudioSubsystemProxy *__fastcall AudioSubsystemConstructor(AudioSubsystemProxy *_this) {
  _this->proxy = new AudioSubsystem();
  return _this;
}
void __fastcall AudioSubsystemDestructor(AudioSubsystemProxy *_this) { delete _this->proxy; }
void *__fastcall AudioSubsystemGetDigitalDriver(AudioSubsystemProxy *_this) { return _this->proxy->GetDigitalDriver(); }
void __fastcall AudioSubsystemCloseDigitalDriver(AudioSubsystemProxy *_this) { _this->proxy->CloseDigitalDriver(); }
void __fastcall AudioSubsystemApplyMidiVolume(AudioSubsystemProxy *_this) { _this->proxy->ApplyMidiVolume(); }
uint32_t __fastcall AudioSubsystemGetActiveSequenceCount(AudioSubsystemProxy *_this) {
  return _this->proxy->GetActiveSequenceCount();
}

AudioSample::AudioSample() {}
AudioSample::~AudioSample() {}
void AudioSample::Start() {}
uint32_t AudioSample::GetIsPlaying() { return 0; }
void AudioSample::SetFade(int32_t rate, int32_t max, int32_t startVol, int32_t endVol) {}
void AudioSample::DoFade() {}
void AudioSample::EnableLoop() {}
void AudioSample::SetLoopCount(int32_t loopCount) {}
void AudioSample::SetVolume(int32_t volume) {}

AudioSampleProxy *__fastcall AudioSampleConstructor(AudioSampleProxy *_this, void *unused,
                                                    AudioSubsystem *audioSubsystem, void *audioData,
                                                    int32_t audioDataSize) {
  _this->proxy = new AudioSample();
  return _this;
}
void __fastcall AudioSampleDestructor(AudioSampleProxy *_this) { delete _this->proxy; }
void __fastcall AudioSampleStart(AudioSampleProxy *_this) { _this->proxy->Start(); }
uint32_t __fastcall AudioSampleGetIsPlaying(AudioSampleProxy *_this) { return _this->proxy->GetIsPlaying(); }
void __fastcall AudioSampleSetFade(AudioSampleProxy *_this, void *unused, int32_t rate, int32_t max, int32_t startVol,
                                   int32_t endVol) {
  _this->proxy->SetFade(rate, max, startVol, endVol);
}
void __fastcall AudioSampleDoFade(AudioSampleProxy *_this) { _this->proxy->DoFade(); }
void __fastcall AudioSampleEnableLoop(AudioSampleProxy *_this) { _this->proxy->EnableLoop(); }
void __fastcall AudioSampleSetLoopCount(AudioSampleProxy *_this, void *unused, int32_t loopCount) {
  _this->proxy->SetLoopCount(loopCount);
}
void __fastcall AudioSampleSetVolume(AudioSampleProxy *_this, void *unused, int32_t volume) {
  _this->proxy->SetVolume(volume);
}

MidiSequence::MidiSequence(AudioSubsystem *audioSubsystem, void *audioData, int32_t audioDataSize) {}
MidiSequence::~MidiSequence() {}
void MidiSequence::Start() {}
void MidiSequence::Stop() {}
void MidiSequence::ApplyCurrentVolume() {}
void MidiSequence::SetVolume(int32_t volume) {}
void MidiSequence::SetLoopCount(int32_t loopCount) {}
uint32_t MidiSequence::GetGlobalActiveSequenceCount() { return 0; }

MidiSequenceProxy *__fastcall MidiSequenceConstructor(MidiSequenceProxy *_this, void *unused,
                                                      AudioSubsystem *audioSubsystem, void *audioData,
                                                      int32_t audioDataSize) {
  _this->proxy = new MidiSequence(audioSubsystem, audioData, audioDataSize);
  return _this;
}
void __fastcall MidiSequenceDestructor(MidiSequenceProxy *_this) { delete _this->proxy; }
void __fastcall MidiSequenceStart(MidiSequenceProxy *_this) { _this->proxy->Start(); }
void __fastcall MidiSequenceStop(MidiSequenceProxy *_this) { _this->proxy->Stop(); }
void __fastcall MidiSequenceApplyCurrentVolume(MidiSequenceProxy *_this) { _this->proxy->ApplyCurrentVolume(); }
void __fastcall MidiSequenceSetVolume(MidiSequenceProxy *_this, void *unused, int32_t volume) {
  _this->proxy->SetVolume(volume);
}
void __fastcall MidiSequenceSetLoopCount(MidiSequenceProxy *_this, void *unused, int32_t loopCount) {
  _this->proxy->SetLoopCount(loopCount);
}
uint32_t __fastcall MidiSequenceGetGlobalActiveSequenceCount(MidiSequenceProxy *_this) {
  return _this->proxy->GetGlobalActiveSequenceCount();
}