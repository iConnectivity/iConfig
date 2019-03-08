/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIXERPARM_H
#define MIXERPARM_H

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

struct MixerParm {
  struct MixerBlock {
    roByte mixerConfigurationNumber;
    roByte maximumInputs;
    roByte maximumOutputs;
  };

  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Byte audioConfigurationNumber);
  static CmdEnum retCommand();

  MixerParm();

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  rwByte activeMixerConfigurationBlock;
  roByte audioConfigurationNumber;
  roByte mixerBlockCount;
  std::vector<MixerBlock> mixerBlocks;
};  // struct MixerParm

struct GetMixerParmCommand
    : public BytesSysexCommand<Command::GetMixerParm> {
  GetMixerParmCommand(DeviceID deviceID, Word transID,
                      Byte audioConfigurationNumber)
      : BytesSysexCommand(deviceID, transID) {
    data.append(audioConfigurationNumber);
  }
};  // GetMixerParmCommand

typedef SysexCommand<Command::SetMixerParm,
                     MixerParm> SetMixerParmCommand;

}  // namespace GeneSysLib

#endif // MIXERPARM_H
