/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXERINPUTCONTROLVALUE_H
#define MIXERINPUTCONTROLVALUE_H

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

struct MixerInputControlValue {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word audioPortID, Byte mixerOutputNumber, Byte mixerInputNumber);
  static CmdEnum retCommand();

  MixerInputControlValue();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roByte mixerOutputNumber;
  roByte mixerInputNumber;
  std::bitset<8> includedFlags;
  rwWord volumeControl;
  rwByte muteControl;
  rwByte soloControl;
  rwByte soloPFLControl;
  rwByte stereoLinkControl;
  rwByte invertControl;
  rwWord panControl;
  rwByte panCurveLaw;

};  // struct MixerInputControlValue

struct GetMixerInputControlValueCommand
    : public BytesSysexCommand<Command::GetMixerInputControlValue> {
  GetMixerInputControlValueCommand(DeviceID deviceID, Word transID,
                                    Word audioPortID,
                                    Byte mixerOutputNumber, Byte mixerInputNumber)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
    data.append(mixerOutputNumber);
    data.append(mixerInputNumber);
  }
};  // GetMixerInputControlValueCommand

typedef SysexCommand<Command::SetMixerInputControlValue,
                     MixerInputControlValue> SetMixerInputControlValueCommand;

}  // namespace GeneSysLib

#endif // MIXERINPUTCONTROLVALUE_H
