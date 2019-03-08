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
#include "MixerInputInterface.h"
#include "MixerMeterValue.h"

using namespace std;
using namespace GeneSysLib;


// Constructor/destructor
MixerInputInterface::MixerInputInterface(DeviceInfoPtr device)
    : device(device) {}

MixerInputInterface::~MixerInputInterface() {}

bool MixerInputInterface::isInvertAvailable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.invertExists();
}

bool MixerInputInterface::isInvertEditable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.invertEditable();
}

bool MixerInputInterface::invert(Word audioPortID, Byte outChannelID, Byte inChannelID) const
{
  const auto& mixerInputControl = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  return mixerInputControl.invertControl() == 1;
}

void MixerInputInterface::invert(Word audioPortID, Byte outChannelID, Byte inChannelID, bool value)
{
  auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  mixerInputControlValue.invertControl(value);
  device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
  //printf("set invert on %d:%d:%d to %d\n", audioPortID, outChannelID, inChannelID, value);
}

bool MixerInputInterface::isStereoLinkAvailable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.stereoLinkExists();

}

bool MixerInputInterface::isStereoLinkEditable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.stereoLinkEditable();

}

bool MixerInputInterface::stereoLink(Word audioPortID, Byte outChannelID, Byte inChannelID) const
{
  const auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  return mixerInputControlValue.stereoLinkControl() == 1;

}

void MixerInputInterface::stereoLink(Word audioPortID, Byte outChannelID, Byte inChannelID, bool value)
{
  if (!(inChannelID % 2)) { // is even
    inChannelID = inChannelID - 1;
  }

  auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  mixerInputControlValue.stereoLinkControl(value);
  device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);

  /*int linkedInChannelID = 0;
  if (inChannelID % 2) { // is odd
    linkedInChannelID = inChannelID + 1;
  }
  else { //even
    linkedInChannelID = inChannelID - 1;
  }

  auto& mixerInputControlValue2 = device->get<MixerInputControlValue>(audioPortID, outChannelID, linkedInChannelID);
  mixerInputControlValue2.stereoLinkControl(value);
  if (value) {
    mixerInputControlValue2.muteControl(mute(audioPortID,outChannelID,inChannelID));
    mixerInputControlValue2.soloControl(solo(audioPortID,outChannelID,inChannelID));
    mixerInputControlValue2.soloPFLControl(soloPFL(audioPortID,outChannelID,inChannelID));
    mixerInputControlValue2.panControl(panCurrent(audioPortID,outChannelID,inChannelID));
    mixerInputControlValue2.volumeControl(volumeCurrent(audioPortID,outChannelID,inChannelID));
    mixerInputControlValue2.panCurveLaw(panCurveLawCurrent(audioPortID,outChannelID,inChannelID));
  }
  device->send<SetMixerInputControlValueCommand>(mixerInputControlValue2);*/
}

bool MixerInputInterface::isSoloPFLAvailable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.soloPFLExists();

}

bool MixerInputInterface::isSoloPFLEditable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.soloPFLEditable();

}

bool MixerInputInterface::soloPFL(Word audioPortID, Byte outChannelID, Byte inChannelID) const
{
  const auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  return mixerInputControlValue.soloPFLControl() == 1;

}

void MixerInputInterface::soloPFL(Word audioPortID, Byte outChannelID, Byte inChannelID, bool value)
{
  auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  mixerInputControlValue.soloPFLControl(value);
  device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);

  /*if (mixerInputControlValue.stereoLinkControl() == 1) {
    int linkedInChannelID = inChannelID + 1;

    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // stereo in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, linkedInChannelID);
      mixerInputControlValue.soloPFLControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
    else { // stereo in / mono out
      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, linkedInChannelID);
      mixerInputControlValue.soloPFLControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }
  else { // mono inputs
    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // mono in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, inChannelID);
      mixerInputControlValue.soloPFLControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }*/
}

bool MixerInputInterface::isSoloAvailable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.soloExists();

}

bool MixerInputInterface::isSoloEditable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.stereoLinkEditable();

}

bool MixerInputInterface::solo(Word audioPortID, Byte outChannelID, Byte inChannelID) const
{
  const auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  return mixerInputControlValue.soloControl() == 1;

}

void MixerInputInterface::solo(Word audioPortID, Byte outChannelID, Byte inChannelID, bool value)
{
  auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  mixerInputControlValue.soloControl(value);
  device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);

  /*if (mixerInputControlValue.stereoLinkControl() == 1) {
    int linkedInChannelID = inChannelID + 1;

    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // stereo in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, linkedInChannelID);
      mixerInputControlValue.soloControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
    else { // stereo in / mono out
      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, linkedInChannelID);
      mixerInputControlValue.soloControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }
  else { // mono inputs
    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // mono in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, inChannelID);
      mixerInputControlValue.soloControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }*/
}

bool MixerInputInterface::isMuteAvailable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.muteExists();

}

bool MixerInputInterface::isMuteEditable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.muteEditable();

}

bool MixerInputInterface::mute(Word audioPortID, Byte outChannelID, Byte inChannelID) const
{
  const auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  return mixerInputControlValue.muteControl() == 1;

}

void MixerInputInterface::mute(Word audioPortID, Byte outChannelID, Byte inChannelID, bool value)
{
  auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  mixerInputControlValue.muteControl(value);
  device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);

  /*if (mixerInputControlValue.stereoLinkControl() == 1) {
    int linkedInChannelID = inChannelID + 1;

    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // stereo in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, linkedInChannelID);
      mixerInputControlValue.muteControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
    else { // stereo in / mono out
      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, linkedInChannelID);
      mixerInputControlValue.muteControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }
  else { // mono inputs
    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // mono in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, inChannelID);
      mixerInputControlValue.muteControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }*/
}

bool MixerInputInterface::isVolumeAvailable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.volumeExists();

}

bool MixerInputInterface::isVolumeEditable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.volumeEditable();

}

int16_t MixerInputInterface::volumeMin(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.minimumVolumeControl();

}

int16_t MixerInputInterface::volumeMax(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.maximumVolumeControl();

}

int16_t MixerInputInterface::volumeResolution(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.resolutionVolumeControl();

}

int16_t MixerInputInterface::volumeCurrent(Word audioPortID, Byte outChannelID, Byte inChannelID) const
{
  const auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  return mixerInputControlValue.volumeControl();

}

void MixerInputInterface::volumeCurrent(Word audioPortID, Byte outChannelID, Byte inChannelID, int16_t value)
{
  auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  mixerInputControlValue.volumeControl(value);
  device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);

  /*if (mixerInputControlValue.stereoLinkControl() == 1) {
    int linkedInChannelID = inChannelID + 1;

    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // stereo in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, linkedInChannelID);
      mixerInputControlValue.volumeControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
    else { // stereo in / mono out
      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, linkedInChannelID);
      mixerInputControlValue.volumeControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }
  else { // mono inputs
    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // mono in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, inChannelID);
      mixerInputControlValue.volumeControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }*/

}

int16_t MixerInputInterface::meterCurrent(Word audioPortID, Byte outChannelID, Byte inChannelID) const
{
  const auto& mixerMeterValue = device->get<MixerMeterValue>(audioPortID, outChannelID);
  return mixerMeterValue.inputMeter(inChannelID);
}

bool MixerInputInterface::isPanAvailable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.panExists();

}

bool MixerInputInterface::isPanEditable(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.panEditable();

}

int16_t MixerInputInterface::panMin(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return 0 - mixerInputControl.maximumPanControl();

}

int16_t MixerInputInterface::panMax(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.maximumPanControl();

}

int16_t MixerInputInterface::panCurrent(Word audioPortID, Byte outChannelID, Byte inChannelID) const
{
  const auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  return mixerInputControlValue.panControl();

}

void MixerInputInterface::panCurrent(Word audioPortID, Byte outChannelID, Byte inChannelID, int16_t value)
{
  auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  mixerInputControlValue.panControl(value);
  device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);

  /*if (mixerInputControlValue.stereoLinkControl() == 1) {
    int linkedInChannelID = inChannelID + 1;

    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // stereo in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, linkedInChannelID);
      mixerInputControlValue.panControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
    else { // stereo in / mono out
      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, linkedInChannelID);
      mixerInputControlValue.panControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }
  else { // mono inputs
    const auto& mixerOutputControl = device->get<MixerOutputControlValue>(audioPortID, outChannelID);
    if (mixerOutputControl.stereoLinkControl() == 1) { // mono in / stereo out
      int linkedOutChannelID = outChannelID + 1;

      auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, linkedOutChannelID, inChannelID);
      mixerInputControlValue.panControl(value);
      device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);
    }
  }*/

}

int8_t MixerInputInterface::numPanCurveLaws(Word audioPortID) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.numberPanCurveLaws();
}

int8_t MixerInputInterface::panCurveLawAt(Word audioPortID, Byte whichCurveLaw) const
{
  const auto& mixerInputControl = device->get<MixerInputControl>(audioPortID);
  return mixerInputControl.panCurveLaws.at(whichCurveLaw)();

}

int8_t MixerInputInterface::panCurveLawCurrent(Word audioPortID, Byte outChannelID, Byte inChannelID) const
{
  const auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  return mixerInputControlValue.panCurveLaw();
}

void MixerInputInterface::panCurveLawCurrent(Word audioPortID, Byte outChannelID, Byte inChannelID, int8_t value)
{
  auto& mixerInputControlValue = device->get<MixerInputControlValue>(audioPortID, outChannelID, inChannelID);
  mixerInputControlValue.panCurveLaw(value);
  device->send<SetMixerInputControlValueCommand>(mixerInputControlValue);

}
