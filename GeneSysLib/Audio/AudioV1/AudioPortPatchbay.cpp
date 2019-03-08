/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "AudioPortPatchbay.h"
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

commandDataKey_t AudioPortPatchbay::minKey() {
  return generateKey(Command::RetAudioPortPatchbay);
}

commandDataKey_t AudioPortPatchbay::maxKey() {
  return queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t AudioPortPatchbay::queryKey(Word portID) {
  return generateKey(Command::RetAudioPortPatchbay, portID);
}

CmdEnum AudioPortPatchbay::retCommand() {
  return Command::RetAudioPortPatchbay;
}

CmdEnum AudioPortPatchbay::setCommand() {
  return Command::SetAudioPortPatchbay;
}

Bytes AudioPortPatchbay::ConfigBlock::generate() const {
  Bytes result;

  result += inputChannelNumber();
  result += outputChannelNumber();

  appendMidiWord(result, portIDOfOutput());

  return result;
}

void AudioPortPatchbay::ConfigBlock::parse(BytesIter &beginIter,
                                           BytesIter &endIter) {
  inputChannelNumber = roByte(nextMidiByte(beginIter, endIter));
  outputChannelNumber = rwByte(nextMidiByte(beginIter, endIter));
  portIDOfOutput = rwWord(nextMidiWord(beginIter, endIter));
}

AudioPortPatchbay::AudioPortPatchbay(void) : configBlocks() {}

const commandDataKey_t AudioPortPatchbay::key() const {
  return generateKey(Command::RetAudioPortPatchbay, portID());
}

Bytes AudioPortPatchbay::generate() const {
  Bytes result;

  result += versionNumber() & 0x7F;
  appendMidiWord(result, portID());
  result += configBlocks.size() & 0x7F;

  for (const auto &config : configBlocks) {
    push_back(result, config.generate());
  }

  return result;
}

void AudioPortPatchbay::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    portID = roWord(nextMidiWord(beginIter, endIter));

    Byte numBlocks = nextMidiByte(beginIter, endIter);

    configBlocks.clear();
    for (auto i = 0; i < numBlocks; ++i) {
      ConfigBlock config;
      config.parse(beginIter, endIter);
      configBlocks.push_back(config);
    }
  }
}

Byte AudioPortPatchbay::versionNumber() const { return 0x01; }

AudioPortPatchbay::ConfigBlock &AudioPortPatchbay::findInputBlock(
    Byte inputChannelNumber) {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [inputChannelNumber](AudioPortPatchbay::ConfigBlock block) {
        return (block.inputChannelNumber() == inputChannelNumber);
      });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find input block");
  }
  return *match;
}

const AudioPortPatchbay::ConfigBlock &AudioPortPatchbay::findInputBlock(
    Byte inputChannelNumber) const {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [inputChannelNumber](AudioPortPatchbay::ConfigBlock block) {
        return (block.inputChannelNumber() == inputChannelNumber);
      });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find input block");
  }
  return *match;
}

AudioPortPatchbay::ConfigBlock &AudioPortPatchbay::findOutputBlock(
    Byte outputChannelNumber) {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [outputChannelNumber](AudioPortPatchbay::ConfigBlock block) {
        return (block.outputChannelNumber() == outputChannelNumber);
      });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find output block");
  }
  return *match;
}

const AudioPortPatchbay::ConfigBlock &AudioPortPatchbay::findOutputBlock(
    Byte outputChannelNumber) const {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [outputChannelNumber](AudioPortPatchbay::ConfigBlock block) {
        return (block.outputChannelNumber() == outputChannelNumber);
      });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find output block");
  }
  return *match;
}

size_t AudioPortPatchbay::indexOfInputBlock(Byte inputChannelNumber) const {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [inputChannelNumber](AudioPortPatchbay::ConfigBlock block) {
        return (block.inputChannelNumber() == inputChannelNumber);
      });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find input block");
  }
  return std::distance(configBlocks.begin(), match);
}

size_t AudioPortPatchbay::indexOfOutputBlock(Byte outputChannelNumber) const {
  auto match =
      find_if(configBlocks.begin(), configBlocks.end(),
              [outputChannelNumber](AudioPortPatchbay::ConfigBlock block) {
        return (block.outputChannelNumber() == outputChannelNumber);
      });

  if (match == configBlocks.end()) {
    throw blockNotFoundException("Could not find output block");
  }
  return std::distance(configBlocks.begin(), match);
}

std::vector<FlatAudioPortPatchbay> AudioPortPatchbay::flatList() const {
  std::vector<FlatAudioPortPatchbay> result;

  for (const auto &block : configBlocks) {
    FlatAudioPortPatchbay flatPatchbay = {portID(), block.inputChannelNumber(),
                                          block.outputChannelNumber(),
                                          block.portIDOfOutput()};

    result.push_back(flatPatchbay);
  }

  return result;
}

FlatAudioPortPatchbay AudioPortPatchbay::flatPatchbay(
    Byte inChannelNumber) const {
  const auto &block = findInputBlock(inChannelNumber);
  FlatAudioPortPatchbay flatPatchbay = {portID(), block.inputChannelNumber(),
                                        block.outputChannelNumber(),
                                        block.portIDOfOutput()};
  return flatPatchbay;
}

}  // namespace GeneSysLib
