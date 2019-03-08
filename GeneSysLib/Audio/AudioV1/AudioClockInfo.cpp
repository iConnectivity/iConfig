/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "AudioClockInfo.h"
#include "Generator.h"

using namespace std;
using namespace boost;
using namespace boost::range;

namespace GeneSysLib {

CmdEnum AudioClockInfo::retCommand() { return Command::RetAudioClockInfo; }
CmdEnum AudioClockInfo::setCommand() { return Command::SetAudioClockInfo; }

Bytes AudioClockInfo::SourceBlock::generate() const {
  Bytes result;

  result += sourceNumber();
  result += static_cast<Byte>(sourceType());
  result += sourceInfo.common.number;
  result += sourceInfo.common.reserved;

  return result;
}

void AudioClockInfo::SourceBlock::parse(BytesIter &begin, BytesIter &end) {
  sourceNumber = roByte(nextMidiByte(begin, end));
  sourceType =
      roClockSourceEnum(static_cast<ClockSourceEnum>(nextMidiByte(begin, end)));
  sourceInfo.common.number = nextMidiByte(begin, end);
  sourceInfo.common.reserved = nextMidiByte(begin, end);
}

string AudioClockInfo::SourceBlock::toString() const {
  string result;

  switch (sourceType()) {
    case ClockSource::internal:
      result = str(boost::format("%d - %s Clock") % static_cast<int>(sourceNumber()) %
                   ClockSource::toString(sourceType()));
      break;
    case ClockSource::portClock:
      result =
          str(boost::format("%d - %s %d Clock") % static_cast<int>(sourceNumber()) %
              ClockSource::toString(sourceType()) %
              static_cast<int>(sourceInfo.portClock.portNumber));
      break;
  }
  return result;
}

AudioClockInfo::AudioClockInfo(void) : activeSourceBlock(), m_sourceBlocks() {}

const commandDataKey_t AudioClockInfo::key() const {
  return generateKey(Command::RetAudioClockInfo);
}

Bytes AudioClockInfo::generate() const {
  Bytes result;
  result += static_cast<Byte>(versionNumber() & 0x7F);
  result += activeSourceBlock();
  result += static_cast<Byte>(m_sourceBlocks.size());
  for (const auto &sb : m_sourceBlocks | map_values) {
    push_back(result, sb.generate());
  }
  return result;
}

void AudioClockInfo::parse(BytesIter &beginIter, BytesIter &endIter) {
  const auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    activeSourceBlock = rwByte(nextMidiByte(beginIter, endIter));
    const auto numBlocks = nextMidiByte(beginIter, endIter);
    for (auto i = 0; i < numBlocks; ++i) {
      SourceBlock tempBlock;
      tempBlock.parse(beginIter, endIter);
      m_sourceBlocks[tempBlock.sourceNumber()] = tempBlock;
    }
  }
}

Byte AudioClockInfo::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib
