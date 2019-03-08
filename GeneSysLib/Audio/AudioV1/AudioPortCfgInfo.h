/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOPORTCFGINFODATA_H__
#define __AUDIOPORTCFGINFODATA_H__

#include "BytesCommandData.h"
#include "CommandDataKey.h"
#include "LibTypes.h"
#include "SysexCommand.h"
#include "property.h"

namespace GeneSysLib {

struct AudioPortCfgInfo {
  static commandDataKey_t minKey();
  static commandDataKey_t maxKey();
  static commandDataKey_t queryKey(Word portID);
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  typedef struct ConfigBlock {
    Bytes generate() const;
    void parse(BytesIter &begin, BytesIter &end);
    bool isInputSelectable() const;
    bool isOutputSelectable() const;

    roByte audioCfgNumber;  // From AudioCfgInfo
    roByte minInputChannels;
    roByte maxInputChannels;
    roByte minOutputChannels;
    roByte maxOutputChannels;
  } ConfigBlock;

  AudioPortCfgInfo(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roWord portID;
  rwByte numInputChannels;
  rwByte numOutputChannels;

  const ConfigBlock &block_at(size_t index) const;

  size_t numConfigBlocks() const;

  Byte totalChannels() const;

 private:
  std::vector<ConfigBlock> m_configBlocks;

};  // struct AudioPortCfgInfo

struct GetAudioPortCfgInfoCommand
    : public BytesSysexCommand<Command::GetAudioPortCfgInfo> {
  GetAudioPortCfgInfoCommand(DeviceID deviceID, Word transID, Word portID)
      : BytesSysexCommand(deviceID, transID) {
    data.append(portID);
  }
};

typedef SysexCommand<Command::SetAudioPortCfgInfo, AudioPortCfgInfo>
    SetAudioPortCfgInfoCommand;

}  // namespace GeneSysLib

#endif  // __AUDIOPORTCFGINFODATA_H__
