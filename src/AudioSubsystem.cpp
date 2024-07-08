#include "AudioSubsystem.h"

AudioSubsystem::AudioSubsystem() {
  this->currentMidiSequence = nullptr;
  this->digitalInitialized = false;
  this->soloud = new SoLoud::Soloud();
}

AudioSubsystem::~AudioSubsystem() {
  CloseDigitalDriver();
  delete this->soloud;
}

SoLoud::Soloud *AudioSubsystem::GetDigitalDriver() {
  if (!this->digitalInitialized) {
    this->soloud->init();
    this->digitalInitialized = true;
  }
  return this->soloud;
}

void AudioSubsystem::CloseDigitalDriver() {
  if (this->digitalInitialized) {
    this->soloud->deinit();
    this->digitalInitialized = false;
  }
}

void AudioSubsystem::ApplyMidiVolume() {
  if (this->currentMidiSequence == nullptr) {
    return;
  }

  this->currentMidiSequence->ApplyCurrentVolume();
}

uint32_t AudioSubsystem::GetActiveSequenceCount() {
  if (this->currentMidiSequence == nullptr) {
    return 0;
  }
  return 0;
}

AudioSample::AudioSample(AudioSubsystem *audioSubsystem, unsigned char *audioData, int32_t audioDataSize) {
  this->subsystem = audioSubsystem;
  this->wave = new SoLoud::Wav;
  this->playHandle = -1;
  this->volume = 0;
  this->initialFadeRate = 0;
  this->fadeRate = 0;
  this->maxFade = 0;
  this->startVolume = 0;
  this->endVolume = 0;
  if (audioSubsystem != nullptr && audioData != nullptr && audioDataSize != 0) {
    auto result = this->wave->loadMem(audioData, audioDataSize, false, false);
  }
}

AudioSample::~AudioSample() { delete this->wave; }
void AudioSample::Start() {
  if (this->wave->getLength() != 0) {
    auto driver = this->subsystem->GetDigitalDriver();
    this->playHandle = driver->play(*this->wave);
  }
}

uint32_t AudioSample::GetIsPlaying() {
  auto driver = this->subsystem->GetDigitalDriver();
  return driver->isValidVoiceHandle(this->playHandle);
}

void AudioSample::SetFade(int32_t rate, int32_t max, int32_t startVol, int32_t endVol) {
  this->initialFadeRate = rate;
  this->fadeRate = rate;
  this->maxFade = max;
  this->startVolume = startVol;
  this->endVolume = endVol;
  this->SetVolume(startVol);
}

void AudioSample::DoFade() {
  if (this->maxFade <= 0) {
    return;
  }

  this->fadeRate--;

  if (this->fadeRate != 0) {
    return;
  }

  this->maxFade--;
  this->fadeRate = this->initialFadeRate;

  if (this->endVolume == this->volume) {
    return;
  }

  if (this->volume < this->endVolume) {
    this->volume++;
  } else {
    this->volume--;
  }

  SetVolume(this->volume);
}

void AudioSample::EnableLoop() { this->SetLoopCount(0); }

void AudioSample::SetLoopCount(int32_t loopCount) {
  auto driver = this->subsystem->GetDigitalDriver();
  bool looping = loopCount == 0;
  this->wave->setLooping(looping);
  driver->setLooping(this->playHandle, looping);
}

void AudioSample::SetVolume(int32_t volume) {
  if (volume > 127) {
    volume = 127;
  }

  if (volume < 0) {
    volume = 0;
  }

  this->volume = volume;
  float vol = ((float)volume) / 127.F;
  this->wave->setVolume(vol);
  auto driver = this->subsystem->GetDigitalDriver();
  driver->setVolume(this->playHandle, vol);
}

MidiSequence::MidiSequence(AudioSubsystem *audioSubsystem, unsigned char *midiData, int32_t midiDataSize) {
  this->subsystem = audioSubsystem;
  this->data = nullptr;
  this->dataSize = 0;
}

MidiSequence::~MidiSequence() {}

void MidiSequence::Start() {}

void MidiSequence::Stop() {}

void MidiSequence::ApplyCurrentVolume() {}

void MidiSequence::SetVolume(int32_t volume) {}

void MidiSequence::SetLoopCount(int32_t loopCount) {}

uint32_t MidiSequence::GetGlobalActiveSequenceCount() { return 0; }

// AudioSubsystem
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

// AudioSample
AudioSampleProxy *__fastcall AudioSampleConstructor(AudioSampleProxy *_this, void *unused,
                                                    AudioSubsystemProxy *audioSubsystemProxy, void *audioData,
                                                    int32_t audioDataSize) {
  _this->proxy = new AudioSample(audioSubsystemProxy->proxy, (unsigned char *)audioData, audioDataSize);
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

// MidiSequence
MidiSequenceProxy *__fastcall MidiSequenceConstructor(MidiSequenceProxy *_this, void *unused,
                                                      AudioSubsystemProxy *audioSubsystemProxy, void *midiData,
                                                      int32_t midiDataSize) {
  _this->proxy = new MidiSequence(audioSubsystemProxy->proxy, (unsigned char *)midiData, midiDataSize);
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