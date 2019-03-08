/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXEROUTPUTPARM_H
#define MIXEROUTPUTPARM_H

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

struct MixerOutputParm {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word audioPortID, Byte mixerOutputNumber);
  static CmdEnum retCommand();

  MixerOutputParm();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;
  roByte mixerOutputNumber;
  rwByte numberOfMixerOutputAssignments;
  std::vector<rwByte> mixerOutputAssignments;
  roByte maximumMixNameLength;
  rwByte mixNameLength;
  std::vector<rwByte> mixName;

};  // struct MixerOutputParm

struct GetMixerOutputParmCommand
    : public BytesSysexCommand<Command::GetMixerOutputParm> {
  GetMixerOutputParmCommand(DeviceID deviceID, Word transID,
                            Word audioPortID, Byte mixerOutputNumber)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioPortID);
    data.append(mixerOutputNumber);
  }
};  // GetMixerOutputParmCommand

typedef SysexCommand<Command::SetMixerOutputParm,
                     MixerOutputParm> SetMixerOutputParmCommand;


}  // namespace GeneSysLib

#endif // MIXEROUTPUTPARM_H
