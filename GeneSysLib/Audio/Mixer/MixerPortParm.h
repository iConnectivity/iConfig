/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXERPORTPARM_H
#define MIXERPORTPARM_H

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

struct MixerPortParm {
  struct AudioPortMixerBlock {
    roWord audioPortID;
    rwByte numInputs;
    rwByte numOutputs;
  };

  //static commandDataKey_t minKey();
  //static commandDataKey_t maxKey();
  //static commandDataKey_t queryKey();
  static CmdEnum retCommand();

  MixerPortParm();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roByte audioPortMixerBlockCount;
  std::vector<AudioPortMixerBlock> audioPortMixerBlocks;
};  // struct MixerPortParm

struct GetMixerPortParmCommand
    : public BytesSysexCommand<Command::GetMixerPortParm> {
  GetMixerPortParmCommand(DeviceID deviceID, Word transID)
      : BytesSysexCommand(deviceID, transID) {
  }
};  // GetMixerPortParmCommand

typedef SysexCommand<Command::SetMixerPortParm,
                     MixerPortParm> SetMixerPortParmCommand;

}  // namespace GeneSysLib

#endif // MIXERPORTPARM_H
