/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MixerInputControl.h"
#include "MixerOutputControl.h"
#include "MixerInputControlValue.h"
#include "MixerOutputControlValue.h"
#include "MixerInputParm.h"
#include "MixerOutputParm.h"
#include "MixerParm.h"
#include "MixerPortParm.h"
#include "MixerOutputInterface.h"
#include "MixerMeterValue.h"

using namespace std;
using namespace GeneSysLib;


// Constructor/destructor
MixerOutputInterface::MixerOutputInterface(DeviceInfoPtr device)
    : device(device) {}

MixerOutputInterface::~MixerOutputInterface() {}

bool MixerOutputInterface::isInvertAvailable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.invertExists();
}

bool MixerOutputInterface::isInvertEditable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.invertEditable();
}

bool MixerOutputInterface::invert(Word audioPortID, Byte outChannelID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  return mixerOutputControl.invertControl() == 1;
}

void MixerOutputInterface::invert(Word audioPortID, Byte outChannelID, bool value)
{
  auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  mixerOutputControlValue.invertControl(value);
  device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue);
}

bool MixerOutputInterface::isStereoLinkAvailable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.stereoLinkExists();

}

bool MixerOutputInterface::isStereoLinkEditable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.stereoLinkEditable();

}

bool MixerOutputInterface::stereoLink(Word audioPortID, Byte outChannelID) const
{
  const auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  return mixerOutputControlValue.stereoLinkControl() == 1;

}

void MixerOutputInterface::stereoLink(Word audioPortID, Byte outChannelID, bool value)
{
  if (!(outChannelID % 2)) { // is even
    outChannelID = outChannelID - 1;
  }
  auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  mixerOutputControlValue.stereoLinkControl(value);
  device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue);

  /*int linkedOutChannelID = 0;
  if (outChannelID % 2) { // is odd
    linkedOutChannelID = outChannelID + 1;
  }
  else { //even
    linkedOutChannelID = outChannelID - 1;
  }

  auto& mixerOutputControlValue2 = device->get<MixerOutputControlValue>(audioPortID, linkedOutChannelID);
  mixerOutputControlValue2.stereoLinkControl(value);
  if (value) {
    mixerOutputControlValue2.muteControl(mute(audioPortID,outChannelID));
    mixerOutputControlValue2.soloControl(solo(audioPortID,outChannelID));
    mixerOutputControlValue2.soloPFLControl(soloPFL(audioPortID,outChannelID));
    mixerOutputControlValue2.panControl(panCurrent(audioPortID,outChannelID));
    mixerOutputControlValue2.volumeControl(volumeCurrent(audioPortID,outChannelID));
    mixerOutputControlValue2.panCurveLaw(panCurveLawCurrent(audioPortID,outChannelID));
  }
  device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue2);
  //printf("sent to %d too\n", linkedOutChannelID);*/
}

bool MixerOutputInterface::isSoloPFLAvailable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.soloPFLExists();

}

bool MixerOutputInterface::isSoloPFLEditable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.soloPFLEditable();

}

bool MixerOutputInterface::soloPFL(Word audioPortID, Byte outChannelID) const
{
  const auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  return mixerOutputControlValue.soloPFLControl() == 1;

}

void MixerOutputInterface::soloPFL(Word audioPortID, Byte outChannelID, bool value)
{
  auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  mixerOutputControlValue.soloPFLControl(value);

  device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue);
  /*if (mixerOutputControlValue.stereoLinkControl() == 1) {
    auto& mixerOutputControlValue2 = device->get<MixerOutputControlValue>(audioPortID, outChannelID + 1);
    mixerOutputControlValue2.soloPFLControl(value);
    device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue2);
  }*/
}

bool MixerOutputInterface::isSoloAvailable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.soloExists();

}

bool MixerOutputInterface::isSoloEditable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.soloEditable();

}

int16_t MixerOutputInterface::solo(Word audioPortID, Byte outChannelID) const
{
  const auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  return mixerOutputControlValue.soloControl();

}

void MixerOutputInterface::solo(Word audioPortID, Byte outChannelID, int16_t value)
{
  auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  mixerOutputControlValue.soloControl(value);
  device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue);

  /*if (mixerOutputControlValue.stereoLinkControl() == 1) {
    auto& mixerOutputControlValue2 = device->get<MixerOutputControlValue>(audioPortID, outChannelID + 1);
    mixerOutputControlValue2.soloControl(value);
    device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue2);
  }*/

}

bool MixerOutputInterface::isMuteAvailable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.muteExists();

}

bool MixerOutputInterface::isMuteEditable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.muteEditable();

}

bool MixerOutputInterface::mute(Word audioPortID, Byte outChannelID) const
{
  const auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  return mixerOutputControlValue.muteControl() == 1;
}

void MixerOutputInterface::mute(Word audioPortID, Byte outChannelID, bool value)
{
  auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  mixerOutputControlValue.muteControl(value);
  device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue);

  /*if (mixerOutputControlValue.stereoLinkControl() == 1) {
    auto& mixerOutputControlValue2 = device->get<MixerOutputControlValue>(audioPortID, outChannelID + 1);
    mixerOutputControlValue2.muteControl(value);
    device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue2);
  }*/
}

bool MixerOutputInterface::isVolumeAvailable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.volumeExists();

}

bool MixerOutputInterface::isVolumeEditable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.volumeEditable();

}

int16_t MixerOutputInterface::volumeMin(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.minimumVolumeControl();

}

int16_t MixerOutputInterface::volumeMax(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.maximumVolumeControl();

}

int16_t MixerOutputInterface::volumeResolution(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.resolutionVolumeControl();

}

int16_t MixerOutputInterface::volumeCurrent(Word audioPortID, Byte outChannelID) const
{
  const auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  return mixerOutputControlValue.volumeControl();

}

void MixerOutputInterface::volumeCurrent(Word audioPortID, Byte outChannelID, int16_t value)
{
  auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  mixerOutputControlValue.volumeControl(value);
  device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue);

  /*if (mixerOutputControlValue.stereoLinkControl() == 1) {
    auto& mixerOutputControlValue2 = device->get<MixerOutputControlValue>(audioPortID, outChannelID + 1);
    mixerOutputControlValue2.volumeControl(value);
    device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue2);
  }*/
}

int16_t MixerOutputInterface::meterCurrent(Word audioPortID, Byte outChannelID)
{
  const auto& mixerMeterValue = device->get<MixerMeterValue>(audioPortID, outChannelID);
  return mixerMeterValue.outputMeter();
}

bool MixerOutputInterface::isPanAvailable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.panExists();

}

bool MixerOutputInterface::isPanEditable(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.panEditable();

}

int16_t MixerOutputInterface::panMin(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return 0 - mixerOutputControl.maximumPanControl();

}

int16_t MixerOutputInterface::panMax(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.maximumPanControl();

}

int16_t MixerOutputInterface::panCurrent(Word audioPortID, Byte outChannelID) const
{
  const auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  return mixerOutputControlValue.panControl();

}

void MixerOutputInterface::panCurrent(Word audioPortID, Byte outChannelID, int16_t value)
{
  auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  mixerOutputControlValue.panControl(value);
  device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue);

  /*if (mixerOutputControlValue.stereoLinkControl() == 1) {
    auto& mixerOutputControlValue2 = device->get<MixerOutputControlValue>(audioPortID, outChannelID + 1);
    mixerOutputControlValue2.panControl(value);
    device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue2);
  }*/

}

int8_t MixerOutputInterface::numPanCurveLaws(Word audioPortID) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.numberPanCurveLaws();
}

int8_t MixerOutputInterface::panCurveLawAt(Word audioPortID, Byte whichCurveLaw) const
{
  const auto& mixerOutputControl = device->get<MixerOutputControl>(audioPortID);
  return mixerOutputControl.panCurveLaws.at(whichCurveLaw)();

}

int8_t MixerOutputInterface::panCurveLawCurrent(Word audioPortID, Byte outChannelID) const
{
  const auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  return mixerOutputControlValue.panCurveLaw();

}

void MixerOutputInterface::panCurveLawCurrent(Word audioPortID, Byte outChannelID, int8_t value)
{
  auto& mixerOutputControlValue = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
  mixerOutputControlValue.panCurveLaw(value);
  device->send<SetMixerOutputControlValueCommand>(mixerOutputControlValue);

}
