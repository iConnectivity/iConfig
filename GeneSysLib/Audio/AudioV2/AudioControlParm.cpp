/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioControlParm.h"
#include "StreamHelpers.h"
#include <limits>

using namespace std;

namespace GeneSysLib {

commandDataKey_t AudioControlParm::minKey() {
  return generateKey(AudioControlParm::retCommand());
}

commandDataKey_t AudioControlParm::maxKey() {
  return queryKey(numeric_limits<Word>::max(), numeric_limits<Byte>::max());
}

commandDataKey_t AudioControlParm::queryKey(Word audioPortID,
                                            Byte controllerNumber) {
  return generateKey(AudioControlParm::retCommand(), audioPortID,
                     controllerNumber);
}

CmdEnum AudioControlParm::retCommand() { return Command::RetAudioControlParm; }

CmdEnum AudioControlParm::setCommand() { return Command::SetAudioControlParm; }

AudioControlParm::AudioControlParm()
    : audioPortID(), controllerNumber(), controllerType(), m_controller() {}

const commandDataKey_t AudioControlParm::key() const {
  return AudioControlParm::queryKey(audioPortID(), controllerNumber());
}

Bytes AudioControlParm::generate() const {
  Bytes result;

  appendMidiByte(result, versionNumber());
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, controllerNumber());
  appendMidiByte(result, static_cast<Byte>(controllerType()));
  appendBytes(result, AudioControllerType::generate(m_controller));

  return result;
}

void AudioControlParm::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    controllerNumber = nextROByte(beginIter, endIter);
    controllerType = roControllerTypeEnum(
        static_cast<ControllerTypeEnum>(nextMidiByte(beginIter, endIter)));
    m_controller =
        AudioControllerType::parse(controllerType(), beginIter, endIter);
  }
}

// properties
Byte AudioControlParm::versionNumber() const { return 0x01; }

bool AudioControlParm::isOfType(ControllerTypeEnum _controllerType) const {
  return (controllerType() == _controllerType);
}

Byte AudioControlParm::numDetails() const {
  Byte result = 0;
  if (isOfType(ControllerType::Selector)) {
    result = selector().numSelectorInputs();
  } else if (isOfType(ControllerType::Feature)) {
    result = feature().numFeatureChannels();
  } else if (isOfType(ControllerType::ClockSource)) {
    result = clockSource().numClockSourceInputs();
  }

  return result;
}

AudioControllerType::Selector &AudioControlParm::selector() {
  assert(controllerType() == ControllerType::Selector);
  return boost::get<AudioControllerType::Selector>(m_controller);
}

const AudioControllerType::Selector &AudioControlParm::selector() const {
  assert(controllerType() == ControllerType::Selector);
  return boost::get<AudioControllerType::Selector>(m_controller);
}

const AudioControllerType::Feature &AudioControlParm::feature() const {
  assert(controllerType() == ControllerType::Feature);
  return boost::get<AudioControllerType::Feature>(m_controller);
}

AudioControllerType::ClockSource &AudioControlParm::clockSource() {
  assert(controllerType() == ControllerType::ClockSource);
  return boost::get<AudioControllerType::ClockSource>(m_controller);
}

const AudioControllerType::ClockSource &AudioControlParm::clockSource() const {
  assert(controllerType() == ControllerType::ClockSource);
  return boost::get<AudioControllerType::ClockSource>(m_controller);
}

std::string AudioControlParm::controllerName() const {
  string result = "";

  switch (controllerType()) {
  case ControllerType::Selector: {
    result = selector().controllerName();
    break;
  }

  case ControllerType::Feature: {
    result = feature().controllerName();
    break;
  }

  case ControllerType::ClockSource: {
    result = clockSource().controllerName();
    break;
  }

  default:
    break;
  }

  return result;
}

}  // namespace GeneSysLib
