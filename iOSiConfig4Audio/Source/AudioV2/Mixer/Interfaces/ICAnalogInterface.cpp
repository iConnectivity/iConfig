/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "ICAnalogInterface.h"
#include "AudioDeviceParm.h"
#include "AudioControlParm.h"
#include "AudioControlDetail.h"
#include "AudioControlDetailValue.h"
#include "AudioControlDetailValueTypes.h"
#include "AudioPortMeterValue.h"

using namespace std;
using namespace GeneSysLib;

const double volumedBConversionFactor = 1.0 / 256.0;

// Constructor/destructor
ICAnalogInterface::ICAnalogInterface(DeviceInfoPtr device,
                                                     Word audioPortID,
                                                     Byte controllerNumber)
    : device(device),
      audioPortID(audioPortID),
      controllerNumber(controllerNumber) {}

ICAnalogInterface::~ICAnalogInterface() {}

// Controller features
string ICAnalogInterface::controllerName() const {
  const AudioControlParm &audioControl =
      device->get<AudioControlParm>(audioPortID, controllerNumber);
  return audioControl.controllerName();
}

Byte ICAnalogInterface::numChannels() const {
  assert(device->contains<AudioControlParm>(audioPortID, controllerNumber));
  const AudioControlParm &audioControl =
      device->get<AudioControlParm>(audioPortID, controllerNumber);
  return audioControl.numDetails();
}

// Channel name
string ICAnalogInterface::channelName(Byte channelID) const {
  assert(device->contains<AudioControlDetail>(audioPortID, controllerNumber,
                                              channelID));
  const AudioControlDetail &audioControl =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControl.channelName();
}

// Stereo Link methods
bool ICAnalogInterface::isStereoLinkAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().stereoLinkControlExists();
}

bool ICAnalogInterface::isStereoLinkEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().stereoLinkControlEditable();
}

bool ICAnalogInterface::stereoLink(Byte channelID) const {
  //NSLog(@"stereoLink: %d, %d, %d", audioPortID, controllerNumber, channelID);
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  return audioControlDetailValue.feature().stereoLink();
}

void ICAnalogInterface::stereoLink(Byte channelID, bool value) {
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
bool ICAnalogInterface::isHighImpedanceAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().highImpedanceControlExists();
}

bool ICAnalogInterface::isHighImpedanceEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().highImpedanceControlEditable();
}

bool ICAnalogInterface::highImpedance(Byte channelID) const {
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  return audioControlDetailValue.feature().highImpedance();
}

void ICAnalogInterface::highImpedance(Byte channelID, bool value) {
  AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  audioControlDetailValue.feature().highImpedance(value);
  device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);
}

// Phantom power methods
bool ICAnalogInterface::isPhantomPowerAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().phantomPowerExists();
}

bool ICAnalogInterface::isPhantomPowerEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().phantomPowerEditable();
}

bool ICAnalogInterface::phantomPower(Byte channelID) const {
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  return audioControlDetailValue.feature().phantomPower() == 0x01;
}

void ICAnalogInterface::phantomPower(Byte channelID, bool value) {
  AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  audioControlDetailValue.feature().phantomPower(value);
  device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);
}

// Mute methods
bool ICAnalogInterface::isMuteAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().muteControlExists();
}

bool ICAnalogInterface::isMuteEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().muteControlEditable();
}

bool ICAnalogInterface::mute(Byte channelID) const {
  const AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);
  return audioControlDetailValue.feature().mute();
}

void ICAnalogInterface::mute(Byte channelID, bool value) {
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

int8_t ICAnalogInterface::channelType(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().channelType();
}

int8_t ICAnalogInterface::channelNumber(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().channelNumber();
}

// Volume methods
bool ICAnalogInterface::isVolumeAvailable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().volumeControlExists();
}

bool ICAnalogInterface::isVolumeEditable(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return audioControlDetail.feature().volumeControlEditable();
}

int16_t ICAnalogInterface::volumeMin(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().minimumVolumeControl();
}

int16_t ICAnalogInterface::volumeMax(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().maximumVolumeControl();
}

int16_t ICAnalogInterface::trimMin(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().minimumTrimControl();
}

int16_t ICAnalogInterface::trimMax(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().maximumTrimControl();
}

int16_t ICAnalogInterface::volumeResolution(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().resolutionVolumeControl();
}

int16_t ICAnalogInterface::volumePad(Byte channelID) const {
  const AudioControlDetail &audioControlDetail =
      device->get<AudioControlDetail>(audioPortID, controllerNumber, channelID);
  return (int16_t) audioControlDetail.feature().padVolumeControl();
}

int16_t ICAnalogInterface::volumeCurrent(Byte channelID) const {
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

void ICAnalogInterface::volumeCurrent(Byte channelID, int16_t value) {
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

int16_t ICAnalogInterface::meterCurrent(Byte channelID) {
  if (device->contains(AudioPortMeterValue::queryKey(audioPortID))) {
    ////printf("Contains <AudioPortMeterValue>\n");
    const AudioPortMeterValue &audioPortMeterValues = device->get<AudioPortMeterValue>(audioPortID);
    int chanType = channelType(channelID) - 1;
    int chanNum = channelNumber(channelID) - 1;
    return audioPortMeterValues.meterValue(chanType, chanNum);
  }
  return 0;
}

int16_t ICAnalogInterface::trimCurrent(Byte channelID) const {
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

void ICAnalogInterface::trimCurrent(Byte channelID, int16_t value) {
  AudioControlDetailValue &audioControlDetailValue =
      device->get<AudioControlDetailValue>(audioPortID, controllerNumber,
                                           channelID);

  assert(audioControlDetailValue.controllerType() == ControllerType::Feature);

  if (audioControlDetailValue.feature().isVolumeIncluded()) {
    audioControlDetailValue.feature().trim(value);
    device->send<SetAudioControlDetailValueCommand>(audioControlDetailValue);
  }
}
