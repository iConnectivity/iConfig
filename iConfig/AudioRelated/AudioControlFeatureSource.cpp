/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioControlFeatureSource.h"
#include "AudioDeviceParm.h"
#include "AudioControlParm.h"
#include "AudioControlDetail.h"
#include "AudioControlDetailValue.h"
#include "AudioControlDetailValueTypes.h"
#include "AudioPortMeterValue.h"

using namespace std;
using namespace GeneSysLib;

const double IAudioControlFeatureSource::VolumedBConversionFactor = 1.0 / 256.0;

// Constructor/destructor
AudioControlFeatureSource::AudioControlFeatureSource(DeviceInfoPtr device,
                                                     Word audioPortID,
                                                     Byte controllerNumber)
    : device(device),
      audioPortID(audioPortID),
      controllerNumber(controllerNumber) {
  ;
}

AudioControlFeatureSource::~AudioControlFeatureSource() {}

// Controller features
string AudioControlFeatureSource::controllerName() const {
  const AudioControlParm &audioControl =
      device->get<AudioControlParm>(audioPortID, controllerNumber);
  return audioControl.controllerName();
}

Byte AudioControlFeatureSource::numChannels() const {
  //PlayAudio support zx 2017-02-22
  bool bHavePort = device->contains<AudioControlParm>(audioPortID, controllerNumber);
  if(bHavePort == false)
          return 0;
  //assert(device->contains<AudioControlParm>(audioPortID, controllerNumber));
  //assert(device->contains<AudioControlParm>(2, controllerNumber));
  const AudioControlParm &audioControl =
      device->get<AudioControlParm>(audioPortID, controllerNumber);
  return audioControl.numDetails();
}

// Channel name
string AudioControlFeatureSource::channelName(Byte channelID) const {
  assert(device->contains<AudioControlDetail>(audioPortID, controllerNumber,
                                              channelID));
  const AudioControlDetail &audioControl =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControl.channelName();
}

// Stereo Link methods
bool AudioControlFeatureSource::isStereoLinkAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().stereoLinkControlExists();
}

bool AudioControlFeatureSource::isStereoLinkEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().stereoLinkControlEditable();
}

bool AudioControlFeatureSource::stereoLink(Byte channelID) const {
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  return audioControlDetailValue.feature().stereoLink();
}

void AudioControlFeatureSource::stereoLink(Byte channelID, bool value) {
  if (!(channelID % 2)) { // is even
    channelID = channelID - 1;
  }

  AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  audioControlDetailValue.feature().stereoLink(value);
  device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);

  // and the partner!

  AudioControlDetailValue &audioControlDetailValue2 =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID + 1);
  audioControlDetailValue2.feature().stereoLink(value);
  audioControlDetailValue2.feature().volume(audioControlDetailValue.feature().volume());
  audioControlDetailValue2.feature().mute(audioControlDetailValue.feature().mute());
  device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue2);
}

// High impedence methods
bool AudioControlFeatureSource::isHighImpedanceAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().highImpedanceControlExists();
}

bool AudioControlFeatureSource::isHighImpedanceEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().highImpedanceControlEditable();
}

bool AudioControlFeatureSource::highImpedance(Byte channelID) const {
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  return audioControlDetailValue.feature().highImpedance();
}

void AudioControlFeatureSource::highImpedance(Byte channelID, bool value) {
  AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  audioControlDetailValue.feature().highImpedance(value);
  device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);
}

// Phantom power methods
bool AudioControlFeatureSource::isPhantomPowerAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().phantomPowerExists();
}

bool AudioControlFeatureSource::isPhantomPowerEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().phantomPowerEditable();
}

bool AudioControlFeatureSource::phantomPower(Byte channelID) const {
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  return audioControlDetailValue.feature().phantomPower() == 0x01;
}

void AudioControlFeatureSource::phantomPower(Byte channelID, bool value) {
  AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  audioControlDetailValue.feature().phantomPower(value);
  device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);
}

// Mute methods
bool AudioControlFeatureSource::isMuteAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().muteControlExists();
}

bool AudioControlFeatureSource::isMuteEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().muteControlEditable();
}

bool AudioControlFeatureSource::mute(Byte channelID) const {
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  return audioControlDetailValue.feature().mute();
}

void AudioControlFeatureSource::mute(Byte channelID, bool value) {
  AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  audioControlDetailValue.feature().mute(value);
  device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);

  if (stereoLink(channelID)) {
    AudioControlDetailValue &audioControlDetailValue =
        device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                             channelID + 1);
    audioControlDetailValue.feature().mute(value);
    device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);
  }
}

int8_t AudioControlFeatureSource::channelType(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().channelType();
}

int8_t AudioControlFeatureSource::channelNumber(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().channelNumber();
}

// Volume methods
bool AudioControlFeatureSource::isVolumeAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().volumeControlExists();
}

bool AudioControlFeatureSource::isVolumeEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().volumeControlEditable();
}

int16_t AudioControlFeatureSource::volumeMin(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().minimumVolumeControl();
}

int16_t AudioControlFeatureSource::volumeMax(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().maximumVolumeControl();
}

int16_t AudioControlFeatureSource::trimMin(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().minimumTrimControl();
}

int16_t AudioControlFeatureSource::trimMax(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().maximumTrimControl();
}

int16_t AudioControlFeatureSource::volumeResolution(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().resolutionVolumeControl();
}

int16_t AudioControlFeatureSource::volumePad(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().padVolumeControl();
}

int16_t AudioControlFeatureSource::volumeCurrent(Byte channelID) const {
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  int16_t value = 0;

  assert(audioControlDetailValue.controllerType() == ControllerType::Feature);

  if (audioControlDetailValue.feature().isVolumeIncluded()) {
    value = audioControlDetailValue.feature().volume();
  }

  return value;
}

void AudioControlFeatureSource::volumeCurrent(Byte channelID, int16_t value) {
  AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);

  assert(audioControlDetailValue.controllerType() == ControllerType::Feature);

  if (audioControlDetailValue.feature().isVolumeIncluded()) {
    audioControlDetailValue.feature().volume(value);
    device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);
  }

  // and partner
  if (stereoLink(channelID)) {
    AudioControlDetailValue &audioControlDetailValue =
        device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                             channelID + 1);

    assert(audioControlDetailValue.controllerType() == ControllerType::Feature);

    if (audioControlDetailValue.feature().isVolumeIncluded()) {
      audioControlDetailValue.feature().volume(value);
      device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);
    }
  }
}

int16_t AudioControlFeatureSource::meterCurrent(Byte channelID) {
  if (device->contains(AudioPortMeterValue::queryKey(audioPortID))) {
    ////printf("Contains <AudioPortMeterValue>\n");
    const AudioPortMeterValue &audioPortMeterValues = device->get<AudioPortMeterValue>(audioPortID);
    return audioPortMeterValues.meterValue(channelType(channelID) - 1, channelNumber(channelID) - 1);
  }
  return 0;
}

int16_t AudioControlFeatureSource::trimCurrent(Byte channelID) const {
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  int16_t value = 0;

  assert(audioControlDetailValue.controllerType() == ControllerType::Feature);

  if (audioControlDetailValue.feature().isVolumeIncluded()) {
    value = audioControlDetailValue.feature().trim();
  }

  return value;
}

void AudioControlFeatureSource::trimCurrent(Byte channelID, int16_t value) {
  AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);

  assert(audioControlDetailValue.controllerType() == ControllerType::Feature);

  if (audioControlDetailValue.feature().isVolumeIncluded()) {
    audioControlDetailValue.feature().trim(value);
    device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);
  }
}
