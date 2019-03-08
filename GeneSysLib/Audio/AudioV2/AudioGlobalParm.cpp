/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioGlobalParm.h"
#include "Generator.h"
#include "SaveRestoreID.h"
#include "StreamHelpers.h"

#ifndef Q_MOC_RUN
#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#endif

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace boost::range;
using namespace boost::io;

namespace GeneSysLib {

CmdEnum AudioGlobalParm::retCommand() { return Command::RetAudioGlobalParm; }

CmdEnum AudioGlobalParm::setCommand() { return Command::SetAudioGlobalParm; }

Bytes AudioGlobalParm::ConfigBlock::generate() const {
  Bytes result;
  result += number();
  result += bitDepth();
  result += sampleRate();

  return result;
}

void AudioGlobalParm::ConfigBlock::parse(BytesIter &beginIter,
                                         BytesIter &endIter) {
  number = roByte(nextMidiByte(beginIter, endIter));
  bitDepth = roBitDepthEnum(
      static_cast<BitDepthEnum>(nextMidiByte(beginIter, endIter)));
  sampleRate = roSampleRateEnum(
      static_cast<SampleRateEnum>(nextMidiByte(beginIter, endIter)));
}

AudioGlobalParm::ConfigBlock AudioGlobalParm::ConfigBlock::parseConfigBlock(
    BytesIter &beginIter, BytesIter &endIter) {
  AudioGlobalParm::ConfigBlock b;
  b.parse(beginIter, endIter);
  return b;
}

std::string AudioGlobalParm::ConfigBlock::toString() const {
  return str(format("%1%Hz, %2%") % number() %
             (SampleRate::toString(sampleRate())) %
             (BitDepth::toString(bitDepth())));
}

AudioGlobalParm::AudioGlobalParm(void)
    : numAudioPorts(),
      minAudioFrames(),
      maxAudioFrames(),
      currentAudioFrames(minAudioFrames, maxAudioFrames),
      minSyncFactor(),
      maxSyncFactor(),
      currentSyncFactor(minSyncFactor, maxSyncFactor),
      currentActiveConfig(),
      m_configBlocks() {}

const commandDataKey_t AudioGlobalParm::key() const {
  return generateKey(Command::RetAudioGlobalParm);
}

Bytes AudioGlobalParm::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, numAudioPorts());
  result += minAudioFrames();
  result += maxAudioFrames();
  result += currentAudioFrames();
  result += minSyncFactor();
  result += maxSyncFactor();
  result += currentSyncFactor();
  result += currentActiveConfig();
  result += static_cast<Byte>(m_configBlocks.size());
  for (const auto &configBlock : m_configBlocks) {
    push_back(result, configBlock.generate());
  }

  return result;
}

void AudioGlobalParm::parse(BytesIter &beginIter, BytesIter &endIter) {
  const auto &version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    numAudioPorts = roWord(nextMidiWord(beginIter, endIter));
    minAudioFrames = roByte(nextMidiByte(beginIter, endIter));
    maxAudioFrames = roByte(nextMidiByte(beginIter, endIter));
    currentAudioFrames = rwByte(nextMidiByte(beginIter, endIter),
                                minAudioFrames, maxAudioFrames);
    minSyncFactor = roByte(nextMidiByte(beginIter, endIter));
    maxSyncFactor = roByte(nextMidiByte(beginIter, endIter));
    currentSyncFactor =
        rwByte(nextMidiByte(beginIter, endIter), minSyncFactor, maxSyncFactor);
    currentActiveConfig = rwByte(nextMidiByte(beginIter, endIter));

    const auto &tempNumConfigBlocks = nextMidiByte(beginIter, endIter);
    m_configBlocks.clear();
    for (auto i = 0; i < tempNumConfigBlocks; ++i) {
      m_configBlocks.push_back(
          AudioGlobalParm::ConfigBlock::parseConfigBlock(beginIter, endIter));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// properties
////////////////////////////////////////////////////////////////////////////////
Byte AudioGlobalParm::versionNumber() const { return 0x01; }

Byte AudioGlobalParm::numConfigBlocks() const {
  return static_cast<Byte>(m_configBlocks.size() & 0xFF);
}

const AudioGlobalParm::ConfigBlock &AudioGlobalParm::configBlock(
    Byte configID) const {
  assert(((int)configID > 0) && ((int)configID <= (int)m_configBlocks.size()));
  return m_configBlocks.at(configID - 1);
}

const AudioGlobalParm::ConfigBlock &AudioGlobalParm::activeConfigBlock() const {
  return configBlock(currentActiveConfig());
}

}  // namespace GeneSysLib
