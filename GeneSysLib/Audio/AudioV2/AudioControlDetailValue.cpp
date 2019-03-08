/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioControlDetailValue.h"
#include "Generator.h"
#include "stdafx.h"
#include <algorithm>
#include <limits>
#ifndef Q_MOC_RUN
#include <boost/range/algorithm_ext.hpp>
#include <boost/range.hpp>
#endif
#include <iterator>

namespace GeneSysLib {

CmdEnum AudioControlDetailValue::retCommand() {
  return Command::RetAudioControlDetailValue;
}

CmdEnum AudioControlDetailValue::setCommand() {
  return Command::SetAudioControlDetailValue;
}

commandDataKey_t AudioControlDetailValue::minKey() {
  return generateKey(Command::RetAudioControlDetailValue);
}

commandDataKey_t AudioControlDetailValue::maxKey() {
  return queryKey(std::numeric_limits<Word>::max(),
                  std::numeric_limits<Byte>::max(),
                  std::numeric_limits<Byte>::max());
}

commandDataKey_t AudioControlDetailValue::queryKey(Word audioPortID,
                                                   Byte controllerNumber,
                                                   Byte detailNumber) {
  return generateKey(Command::RetAudioControlDetailValue, audioPortID,
                     controllerNumber, detailNumber);
}

AudioControlDetailValue::AudioControlDetailValue() {}

const commandDataKey_t AudioControlDetailValue::key() const {
  return generateKey(Command::RetAudioControlDetailValue, audioPortID(),
                     controllerNumber(), detailNumber());
}

Bytes AudioControlDetailValue::generate() const {
  Bytes result;

  appendMidiByte(result, versionNumber());
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, controllerNumber());
  appendMidiByte(result, detailNumber());
  appendMidiByte(result, static_cast<Byte>(controllerType()));
  appendBytes(result, AudioControlDetailValueTypes::generate(m_details));

  return result;
}

void AudioControlDetailValue::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    controllerNumber = nextROByte(beginIter, endIter);
    detailNumber = nextROByte(beginIter, endIter);
    controllerType = roControllerTypeEnum(
        (ControllerType::Enum)nextMidiByte(beginIter, endIter));
    m_details = AudioControlDetailValueTypes::parse(controllerType(), beginIter,
                                                    endIter);
  }
}

Byte AudioControlDetailValue::versionNumber() const { return 0x01; }

const AudioControlDetailValueTypes::Selector &
AudioControlDetailValue::selector() const {
  assert(controllerType() == ControllerType::Selector);
  return boost::get<AudioControlDetailValueTypes::Selector>(m_details);
}

AudioControlDetailValueTypes::Feature &AudioControlDetailValue::feature() {
  assert(controllerType() == ControllerType::Feature);
  return boost::get<AudioControlDetailValueTypes::Feature>(m_details);
}

const AudioControlDetailValueTypes::Feature &AudioControlDetailValue::feature()
    const {
  assert(controllerType() == ControllerType::Feature);
  return boost::get<AudioControlDetailValueTypes::Feature>(m_details);
}

const AudioControlDetailValueTypes::ClockSource
AudioControlDetailValue::clockSource() const {
  assert(controllerType() == ControllerType::ClockSource);
  return boost::get<AudioControlDetailValueTypes::ClockSource>(m_details);
}

}  // namespace GeneSysLib
