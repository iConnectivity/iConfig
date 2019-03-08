/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOCFGINFODATA_H__
#define __AUDIOCFGINFODATA_H__

#include "CommandDataKey.h"
#include "LibTypes.h"
#include "BitDepthCode.h"
#include "SampleRateCode.h"
#include "SysexCommand.h"

namespace GeneSysLib {

struct AudioCfgInfo {
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  struct ConfigBlock {
    // variables
    Byte number;
    BitDepthEnum bitDepthCode;
    SampleRateEnum sampleRateCode;
    Byte numAudioChannels;

    // overloaded Methods
    Bytes generate() const;
    void parse(BytesIter &begin, BytesIter &end);
    static ConfigBlock parseConfigBlock(BytesIter &begin, BytesIter &end);

    // to string
    std::string toString() const;
  };  // struct ConfigBlock

  AudioCfgInfo(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;
  roByte minNumAudioFrames;
  roByte maxNumAudioFrames;
  rwByte currentNumAudioFrames;

  roByte minAllowedSyncFactor;
  roByte maxAllowedSyncFactor;
  rwByte currentSyncFactor;

  rwByte currentActiveConfig;

  Byte numConfigBlocks() const;
  ConfigBlock configBlock(Byte index) const;
  ConfigBlock activeConfigBlock() const;

  std::vector<ConfigBlock> configBlocks;
};  // struct AudioCfgInfo

typedef SysexCommand<Command::GetAudioCfgInfo, EmptyCommandData>
    GetAudioCfgInfoCommand;
typedef SysexCommand<Command::SetAudioCfgInfo, AudioCfgInfo>
    SetAudioCfgInfoCommand;

}  // namespace GeneSysLib

#endif  // __AUDIOCFGINFODATA_H__
