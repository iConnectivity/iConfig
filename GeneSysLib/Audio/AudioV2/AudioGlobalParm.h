/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOGLOBALPARM_H__
#define __AUDIOGLOBALPARM_H__

#include "BitDepthCode.h"
#include "CommandDataKey.h"
#include "LibTypes.h"
#include "SampleRateCode.h"
#include "SysexCommand.h"
#include "property.h"

namespace GeneSysLib {

struct AudioGlobalParm {
  static CmdEnum retCommand();
  static CmdEnum setCommand();

  struct ConfigBlock {
    // overloaded methods
    Bytes generate() const;
    void parse(BytesIter &begin, BytesIter &end);
    static ConfigBlock parseConfigBlock(BytesIter &begin, BytesIter &end);

    // properties
    roByte number;
    roBitDepthEnum bitDepth;
    roSampleRateEnum sampleRate;

    // to string
    std::string toString() const;
  };  // struct ConfigBlock

  AudioGlobalParm(void);

  // overloaded methods
  const commandDataKey_t key() const;
  Bytes generate() const;
  void parse(BytesIter &begin, BytesIter &end);

  // properties
  Byte versionNumber() const;

  roWord numAudioPorts;

  roByte minAudioFrames;
  roByte maxAudioFrames;
  rwByte currentAudioFrames;

  roByte minSyncFactor;
  roByte maxSyncFactor;
  rwByte currentSyncFactor;

  rwByte currentActiveConfig;

  Byte numConfigBlocks() const;
  const ConfigBlock &configBlock(Byte configID) const;
  const ConfigBlock &activeConfigBlock() const;

  template <typename Action>
  void for_each(Action action) {
    for (auto &block : m_configBlocks) {
      action(block);
    }
  }

  template <typename Action>
  void for_each(Action action) const {
    for (auto &block : m_configBlocks) {
      action(block);
    }
  }

 private:
  std::vector<ConfigBlock> m_configBlocks;
};

typedef SysexCommand<Command::GetAudioGlobalParm, EmptyCommandData>
    GetAudioGlobalParmCommand;

typedef SysexCommand<Command::SetAudioGlobalParm, AudioGlobalParm>
    SetAudioGlobalParmCommand;
}
#endif  // __AUDIOGLOBALPARM_H__
