/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXERINPUTPARM_H
#define MIXERINPUTPARM_H

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

struct MixerInputParm {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word audioPortID, Byte mixerInputNumber);
  static CmdEnum retCommand();

  MixerInputParm();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roByte mixerInputNumber;
  rwWord audioSourceAudioPortID;
  rwByte audioSourceChannelID;
};  // struct MixerInputParm

struct GetMixerInputParmCommand
    : public BytesSysexCommand<Command::GetMixerInputParm> {
  GetMixerInputParmCommand(DeviceID deviceID, Word transID,
                                    Word audioPortID, Byte mixerInputNumber)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
    data.append(mixerInputNumber);
  }
};  // GetMixerInputParmCommand

typedef SysexCommand<Command::SetMixerInputParm,
                     MixerInputParm> SetMixerInputParmCommand;

}  // namespace GeneSysLib

#endif // MIXERINPUTPARM_H
