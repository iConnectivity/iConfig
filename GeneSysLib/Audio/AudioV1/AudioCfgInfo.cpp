/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "AudioCfgInfo.h"
#include "Generator.h"
#include "BootMode.h"
#include "SaveRestoreID.h"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace boost::range;
using namespace boost::io;

namespace GeneSysLib {

CmdEnum AudioCfgInfo::retCommand() { return Command::RetAudioCfgInfo; }
CmdEnum AudioCfgInfo::setCommand() { return Command::SetAudioCfgInfo; }

Bytes AudioCfgInfo::ConfigBlock::generate() const {
  Bytes result;
  result += number;
  result += bitDepthCode;
  result += sampleRateCode;
  result += numAudioChannels;

  return result;
}

void AudioCfgInfo::ConfigBlock::parse(BytesIter &beginIter,
                                      BytesIter &endIter) {
  number = nextMidiByte(beginIter, endIter);
  bitDepthCode = static_cast<BitDepthEnum>(nextMidiByte(beginIter, endIter));
  sampleRateCode =
      static_cast<SampleRateEnum>(nextMidiByte(beginIter, endIter));
  numAudioChannels = nextMidiByte(beginIter, endIter);
}

AudioCfgInfo::ConfigBlock AudioCfgInfo::ConfigBlock::parseConfigBlock(
    BytesIter &beginIter, BytesIter &endIter) {
  AudioCfgInfo::ConfigBlock b;
  b.parse(beginIter, endIter);
  return b;
}

string AudioCfgInfo::ConfigBlock::toString() const {
  return str(format("%1%Hz, %2% Channels, %3%") % number %
             (SampleRate::toString(sampleRateCode)) % numAudioChannels %
             (BitDepth::toString(bitDepthCode)));
}

AudioCfgInfo::AudioCfgInfo(void)
    : minNumAudioFrames(0x00),
      maxNumAudioFrames(0xFF),
      currentNumAudioFrames(0x00, minNumAudioFrames, maxNumAudioFrames),
      minAllowedSyncFactor(0x00),
      maxAllowedSyncFactor(0xFF),
      currentSyncFactor(0x01, minAllowedSyncFactor, maxAllowedSyncFactor),
      currentActiveConfig(0x00),
      configBlocks() {}

const commandDataKey_t AudioCfgInfo::key() const {
  return generateKey(Command::RetAudioCfgInfo);
}

Bytes AudioCfgInfo::generate() const {
  Bytes result;

  result += versionNumber();
  result += minNumAudioFrames();
  result += maxNumAudioFrames();
  result += currentNumAudioFrames();
  result += minAllowedSyncFactor();
  result += maxAllowedSyncFactor();
  result += currentSyncFactor();
  result += currentActiveConfig();
  result += (Byte)configBlocks.size();
  for (const auto &configBlock : configBlocks) {
    push_back(result, configBlock.generate());
  }

  return result;
}

void AudioCfgInfo::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    minNumAudioFrames = roByte(nextMidiByte(beginIter, endIter));
    maxNumAudioFrames = roByte(nextMidiByte(beginIter, endIter));
    currentNumAudioFrames = rwByte(nextMidiByte(beginIter, endIter),
                                   minNumAudioFrames, maxNumAudioFrames);
    minAllowedSyncFactor = roByte(nextMidiByte(beginIter, endIter));
    maxAllowedSyncFactor = roByte(nextMidiByte(beginIter, endIter));
    currentSyncFactor = rwByte(nextMidiByte(beginIter, endIter),
                               minAllowedSyncFactor, maxAllowedSyncFactor);
    currentActiveConfig = rwByte(nextMidiByte(beginIter, endIter));

    auto tempNumConfigBlocks = nextMidiByte(beginIter, endIter);
    for (auto i = 0; i < tempNumConfigBlocks; ++i) {
      configBlocks.push_back(
          AudioCfgInfo::ConfigBlock::parseConfigBlock(beginIter, endIter));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// properties
////////////////////////////////////////////////////////////////////////////////
Byte AudioCfgInfo::versionNumber() const { return 0x01; }

Byte AudioCfgInfo::numConfigBlocks() const { return configBlocks.size(); }

AudioCfgInfo::ConfigBlock AudioCfgInfo::configBlock(Byte index) const {
  AudioCfgInfo::ConfigBlock result;
  memset(&result, 0, sizeof(result));

  assert(((int)index > 0) && ((int)index <= (int)configBlocks.size()));

  memcpy(&result, &configBlocks.at(index - 1), sizeof(result));
  return result;
}

AudioCfgInfo::ConfigBlock AudioCfgInfo::activeConfigBlock() const {
  return configBlock(currentActiveConfig());
}

}  // namespace GeneSysLib
