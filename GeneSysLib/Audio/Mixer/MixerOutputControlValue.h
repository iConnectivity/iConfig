/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXEROUTPUTCONTROLVALUE_H
#define MIXEROUTPUTCONTROLVALUE_H

#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "CommandDefines.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "AudioDeviceTypes.h"
#include "property.h"
#include "ControllerType.h"
#include "StreamHelpers.h"

namespace GeneSysLib {

struct MixerOutputControlValue {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word audioPortID, Byte mixerOutputNumber);
  static CmdEnum retCommand();

  MixerOutputControlValue();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roByte mixerOutputNumber;
  std::bitset<8> includedFlags;
  rwWord volumeControl;
  rwByte muteControl;
  rwWord soloControl;
  rwByte soloPFLControl;
  rwByte stereoLinkControl;
  rwByte invertControl;
  rwWord panControl;
  rwByte panCurveLaw;

};  // struct MixerOutputControlValue

struct GetMixerOutputControlValueCommand
    : public BytesSysexCommand<Command::GetMixerOutputControlValue> {
  GetMixerOutputControlValueCommand(DeviceID deviceID, Word transID,
                                    Word audioPortID, Byte mixerOutputNumber)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
    data.append(mixerOutputNumber);
  }
};  // GetMixerOutputControlValueCommand

typedef SysexCommand<Command::SetMixerOutputControlValue,
                     MixerOutputControlValue> SetMixerOutputControlValueCommand;

}  // namespace GeneSysLib

#endif // MIXEROUTPUTCONTROLVALUE_H
