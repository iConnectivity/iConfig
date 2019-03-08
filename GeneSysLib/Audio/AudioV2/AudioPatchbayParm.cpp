/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "AudioPatchbayParm.h"
#include "Generator.h"
#include <iterator>
#ifndef Q_MOC_RUN
#include <boost/range.hpp>
#include <boost/range/algorithm_ext.hpp>
#endif
#include <algorithm>
#include <limits>

using boost::range::push_back;

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t AudioPatchbayParm::minKey() {
  return generateKey(Command::RetAudioPatchbayParm);
}

commandDataKey_t AudioPatchbayParm::maxKey() {
  return queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t AudioPatchbayParm::queryKey(Word _audioPortID) {
  return generateKey(Command::RetAudioPatchbayParm, _audioPortID);
}

CmdEnum AudioPatchbayParm::retCommand() {
  return Command::RetAudioPatchbayParm;
}

CmdEnum AudioPatchbayParm::setCommand() {
  return Command::SetAudioPatchbayParm;
}

Bytes AudioPatchbayParm::ConfigBlock::generate() const {
  Bytes result;

  result += inputChannelNumber();
  result += outputChannelNumber();

  appendMidiWord(result, portIDOfOutput());

  return result;
}

void AudioPatchbayParm::ConfigBlock::parse(BytesIter &beginIter,
                                           BytesIter &endIter) {
  inputChannelNumber = roByte(nextMidiByte(beginIter, endIter));
  outputChannelNumber = rwByte(nextMidiByte(beginIter, endIter));
  portIDOfOutput = rwWord(nextMidiWord(beginIter, endIter));
}

AudioPatchbayParm::AudioPatchbayParm(void) : configBlocks() {}

const commandDataKey_t AudioPatchbayParm::key() const {
  return queryKey(audioPortID());
}

Bytes AudioPatchbayParm::generate() const {
  Bytes result;

  result += versionNumber() & 0x7F;
  appendMidiWord(result, audioPortID());
  result += configBlocks.size() & 0x7F;

  for (const auto &config : configBlocks) {
    push_back(result, config.generate());
  }

  return result;
}

void AudioPatchbayParm::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    audioPortID = roWord(nextMidiWord(beginIter, endIter));

    Byte numBlocks = nextMidiByte(beginIter, endIter);

    configBlocks.clear();
    for (auto i = 0; i < numBlocks; ++i) {
      ConfigBlock config;
      config.parse(beginIter, endIter);
      configBlocks.push_back(config);
    }
  }
}

Byte AudioPatchbayParm::versionNumber() const { return 0x01; }

AudioPatchbayParm::ConfigBlock &AudioPatchbayParm::findInputBlock(
    Byte inputChannelNumber) {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [inputChannelNumber](AudioPatchbayParm::ConfigBlock block) {
    return (block.inputChannelNumber() == inputChannelNumber);
  });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find input block");
  }
  return *match;
}

const AudioPatchbayParm::ConfigBlock &AudioPatchbayParm::findInputBlock(
    Byte inputChannelNumber) const {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [inputChannelNumber](AudioPatchbayParm::ConfigBlock block) {
    return (block.inputChannelNumber() == inputChannelNumber);
  });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find input block");
  }
  return *match;
}

AudioPatchbayParm::ConfigBlock &AudioPatchbayParm::findOutputBlock(
    Byte outputChannelNumber) {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [outputChannelNumber](AudioPatchbayParm::ConfigBlock block) {
    return (block.outputChannelNumber() == outputChannelNumber);
  });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find output block");
  }
  return *match;
}

const AudioPatchbayParm::ConfigBlock &AudioPatchbayParm::findOutputBlock(
    Byte outputChannelNumber) const {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [outputChannelNumber](AudioPatchbayParm::ConfigBlock block) {
    return (block.outputChannelNumber() == outputChannelNumber);
  });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find output block");
  }
  return *match;
}

size_t AudioPatchbayParm::indexOfInputBlock(Byte inputChannelNumber) const {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [inputChannelNumber](AudioPatchbayParm::ConfigBlock block) {
    return (block.inputChannelNumber() == inputChannelNumber);
  });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find input block");
  }
  return std::distance(configBlocks.begin(), match);
}

size_t AudioPatchbayParm::indexOfOutputBlock(Byte outputChannelNumber) const {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [outputChannelNumber](AudioPatchbayParm::ConfigBlock block) {
    return (block.outputChannelNumber() == outputChannelNumber);
  });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find output block");
  }
  return std::distance(configBlocks.begin(), match);
}

std::vector<FlatAudioPatchbayParm> AudioPatchbayParm::flatList() {
  std::vector<FlatAudioPatchbayParm> result;

  for (const auto &block : configBlocks) {
    FlatAudioPatchbayParm flatPatchbay = { audioPortID(),
                                           block.inputChannelNumber(),
                                           block.outputChannelNumber(),
                                           block.portIDOfOutput() };

    result.push_back(flatPatchbay);
  }

  return result;
}

std::vector<FlatAudioPatchbayParm> AudioPatchbayParm::flatList() const {
  std::vector<FlatAudioPatchbayParm> result;

  for (const auto &block : configBlocks) {
    FlatAudioPatchbayParm flatPatchbay = { audioPortID(),
                                           block.inputChannelNumber(),
                                           block.outputChannelNumber(),
                                           block.portIDOfOutput() };

    result.push_back(flatPatchbay);
  }

  return result;
}

FlatAudioPatchbayParm AudioPatchbayParm::flatPatchbay(
    Byte inChannelNumber) const {
  const auto &block = findInputBlock(inChannelNumber);
  FlatAudioPatchbayParm flatPatchbay = { audioPortID(),
                                         block.inputChannelNumber(),
                                         block.outputChannelNumber(),
                                         block.portIDOfOutput() };
  return flatPatchbay;
}

}  // namespace GeneSysLib
