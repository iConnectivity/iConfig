/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioControlDetail.h"

#include "StreamHelpers.h"
#include <limits>

using namespace std;

namespace GeneSysLib {

commandDataKey_t AudioControlDetail::minKey() {
  return generateKey(AudioControlDetail::retCommand());
}

commandDataKey_t AudioControlDetail::maxKey() {
  using namespace std;
  return queryKey(numeric_limits<Word>::max(), numeric_limits<Byte>::max(),
                  numeric_limits<Byte>::max());
}

commandDataKey_t AudioControlDetail::queryKey(Word audioPortID,
                                              Byte controllerNumber,
                                              Byte detailNumber) {
  return generateKey(AudioControlDetail::retCommand(), audioPortID,
                     controllerNumber, detailNumber);
}

CmdEnum AudioControlDetail::retCommand() {
  return Command::RetAudioControlDetail;
}

CmdEnum AudioControlDetail::setCommand() {
  return Command::SetAudioControlDetail;
}

AudioControlDetail::AudioControlDetail()
    : audioPortID(),
      controllerNumber(),
      detailNumber(),
      controllerType(),
      m_details() {}

// overloaded methods
const commandDataKey_t AudioControlDetail::key() const {
  return generateKey(AudioControlDetail::retCommand(), audioPortID(),
                     controllerNumber(), detailNumber());
}

Bytes AudioControlDetail::generate() const {
  Bytes result;

  appendMidiByte(result, versionNumber());
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, controllerNumber());
  appendMidiByte(result, detailNumber());
  appendMidiByte(result, static_cast<Byte>(controllerType()));
  appendBytes(result, AudioControlDetailTypes::generate(m_details));

  return result;
}

void AudioControlDetail::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    controllerNumber = nextROByte(beginIter, endIter);
    detailNumber = nextROByte(beginIter, endIter);
    controllerType = roControllerTypeEnum(
        static_cast<ControllerTypeEnum>(nextMidiByte(beginIter, endIter)));
    m_details =
        AudioControlDetailTypes::parse(controllerType(), beginIter, endIter);
  }
}

// properties
Byte AudioControlDetail::versionNumber() const { return 0x01; }

const AudioControlDetailTypes::Selector &AudioControlDetail::selector() const {
  assert(controllerType() == ControllerType::Selector);
  return boost::get<AudioControlDetailTypes::Selector>(m_details);
}

AudioControlDetailTypes::Feature &AudioControlDetail::feature() {
  assert(controllerType() == ControllerType::Feature);
  return boost::get<AudioControlDetailTypes::Feature>(m_details);
}

const AudioControlDetailTypes::Feature &AudioControlDetail::feature() const {
  assert(controllerType() == ControllerType::Feature);
  return boost::get<AudioControlDetailTypes::Feature>(m_details);
}

const AudioControlDetailTypes::ClockSource
AudioControlDetail::clockSource() const {
  assert(controllerType() == ControllerType::ClockSource);
  return boost::get<AudioControlDetailTypes::ClockSource>(m_details);
}

string AudioControlDetail::channelName() const {
  string result = "";

  switch (controllerType()) {
    case ControllerType::Selector: {
      result = selector().selectorInputName();
      break;
    }

    case ControllerType::Feature: {
      result = feature().channelName();
      break;
    }

    case ControllerType::ClockSource: {
      result = clockSource().clockSourceName();
      break;
    }

    default:
      break;
  }

  return result;
}

// AudioControlDetailTypes::Variants m_details;

}  // namespace GeneSysLib
