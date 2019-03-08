/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOPATCHBAYPARM_H__
#define __AUDIOPATCHBAYPARM_H__

#include "CommandDataKey.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "BytesCommandData.h"
#include "blockNotFound.h"

#include <exception>
#include <string>
#include <vector>
#include "property.h"

namespace GeneSysLib {

struct FlatAudioPatchbayParm {
  Word inPortID;
  Byte inChannelNumber;
  Byte outChannelNumber;
  Word outPortID;
};

struct AudioPatchbayParm {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  struct ConfigBlock {
    Bytes generate() const;
    void parse(BytesIter &beginIter, BytesIter &endIter);

    roByte inputChannelNumber;
    rwByte outputChannelNumber;
    rwWord portIDOfOutput;
  };

  AudioPatchbayParm(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &beginIter, BytesIter &endIter);

  // properties
  Byte versionNumber() const;
  roWord audioPortID;

  ConfigBlock &findInputBlock(Byte inputChannelNumber);
  const ConfigBlock &findInputBlock(Byte inputChannelNumber) const;
  size_t indexOfInputBlock(Byte inputChannelNumber) const;

  ConfigBlock &findOutputBlock(Byte inputChannelNumber);
  const ConfigBlock &findOutputBlock(Byte inputChannelNumber) const;
  size_t indexOfOutputBlock(Byte outputChannelNumber) const;

  std::vector<FlatAudioPatchbayParm> flatList();
  std::vector<FlatAudioPatchbayParm> flatList() const;
  FlatAudioPatchbayParm flatPatchbay(Byte inChannelNumber) const;

 private:
  std::vector<ConfigBlock> configBlocks;
};  // struct AudioPatchbayParm

struct GetAudioPatchbayParmCommand
    : public BytesSysexCommand<Command::GetAudioPatchbayParm> {
  GetAudioPatchbayParmCommand(DeviceID deviceID, Word transID, Word portID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(portID);
  }
};

typedef SysexCommand<Command::SetAudioPatchbayParm, AudioPatchbayParm>
    SetAudioPatchbayParmCommand;

}  // namespace GeneSysLib

#endif  // __AUDIOPATCHBAYPARM_H__
