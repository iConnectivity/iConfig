/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "AudioPortCfgInfo.h"
#include "Generator.h"
#include <limits>

using namespace boost::assign;
using namespace boost::range;

namespace GeneSysLib {

commandDataKey_t AudioPortCfgInfo::minKey() {
  return generateKey(Command::RetAudioPortCfgInfo);
}

commandDataKey_t AudioPortCfgInfo::maxKey() {
  return queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t AudioPortCfgInfo::queryKey(Word portID) {
  return generateKey(Command::RetAudioPortCfgInfo, portID);
}

CmdEnum AudioPortCfgInfo::retCommand() { return Command::RetAudioPortCfgInfo; }

CmdEnum AudioPortCfgInfo::setCommand() { return Command::SetAudioPortCfgInfo; }

Bytes AudioPortCfgInfo::ConfigBlock::generate() const {
  Bytes result;
  result += audioCfgNumber();
  result += minInputChannels();
  result += maxInputChannels();
  result += minOutputChannels();
  result += maxOutputChannels();
  return result;
}

void AudioPortCfgInfo::ConfigBlock::parse(BytesIter &beginIter,
                                          BytesIter &endIter) {
  audioCfgNumber = roByte(nextMidiByte(beginIter, endIter));
  minInputChannels = roByte(nextMidiByte(beginIter, endIter));
  maxInputChannels = roByte(nextMidiByte(beginIter, endIter));
  minOutputChannels = roByte(nextMidiByte(beginIter, endIter));
  maxOutputChannels = roByte(nextMidiByte(beginIter, endIter));
}

bool AudioPortCfgInfo::ConfigBlock::isInputSelectable() const {
  return minInputChannels() != maxInputChannels();
}

bool AudioPortCfgInfo::ConfigBlock::isOutputSelectable() const {
  return minOutputChannels() != maxOutputChannels();
}

AudioPortCfgInfo::AudioPortCfgInfo(void)
    : portID(), numInputChannels(), numOutputChannels(), m_configBlocks() {}

const commandDataKey_t AudioPortCfgInfo::key() const {
  return generateKey(Command::RetAudioPortCfgInfo, portID());
}

Bytes AudioPortCfgInfo::generate() const {
  Bytes result;

  result += versionNumber() & 0x7F;
  appendMidiWord(result, portID());
  result += numInputChannels();
  result += numOutputChannels();
  result += static_cast<Byte>(m_configBlocks.size() & 0x7F);

  for (const auto &block : m_configBlocks) {
    push_back(result, block.generate());
  }

  return result;
}

void AudioPortCfgInfo::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    portID = roWord(nextMidiWord(beginIter, endIter));
    numInputChannels = rwByte(nextMidiByte(beginIter, endIter));
    numOutputChannels = rwByte(nextMidiByte(beginIter, endIter));

    m_configBlocks.clear();
    auto numBlocks = nextMidiByte(beginIter, endIter);
    for (auto i = 0; i < numBlocks; ++i) {
      ConfigBlock block;
      block.parse(beginIter, endIter);
      m_configBlocks.push_back(block);
    }
  }
}

Byte AudioPortCfgInfo::versionNumber() const { return 0x01; }

const AudioPortCfgInfo::ConfigBlock &AudioPortCfgInfo::block_at(
    size_t index) const {
  return m_configBlocks.at(index);
}

size_t AudioPortCfgInfo::numConfigBlocks() const {
  return m_configBlocks.size();
}

Byte AudioPortCfgInfo::totalChannels() const {
  return numInputChannels() + numOutputChannels();
}

}  // namespace GeneSysLib
