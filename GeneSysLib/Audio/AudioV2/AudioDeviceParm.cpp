/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioDeviceParm.h"
#include "CommandDefines.h"
#include "Generator.h"
#include "StreamHelpers.h"
#include "CommandDataKey.h"
#include <limits>

using namespace std;

namespace GeneSysLib {

commandDataKey_t AudioDeviceParm::minKey() {
  return generateKey(AudioDeviceParm::retCommand());
}

commandDataKey_t AudioDeviceParm::maxKey() {
  return queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t AudioDeviceParm::queryKey(Word audioPortID) {
  return generateKey(AudioDeviceParm::retCommand(), audioPortID);
}

CmdEnum AudioDeviceParm::retCommand() { return Command::RetAudioDeviceParm; }

CmdEnum AudioDeviceParm::setCommand() { return Command::SetAudioDeviceParm; }

AudioDeviceParm::AudioDeviceParm()
    : audioPortID(), portType(), maxControllers(), m_details() {}

// overloaded methods
const commandDataKey_t AudioDeviceParm::key() const {
  return generateKey(AudioDeviceParm::retCommand(), audioPortID());
}
Bytes AudioDeviceParm::generate() const {
  Bytes result;

  appendMidiByte(result, versionNumber());
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, static_cast<Byte>(portType()));
  appendMidiByte(result, maxControllers());

  appendBytes(result, AudioDeviceTypes::generate(m_details));

  return result;
}

void AudioDeviceParm::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    portType = roPortTypeEnum(
        static_cast<PortTypeEnum>(nextMidiByte(beginIter, endIter)));
    maxControllers = nextROByte(beginIter, endIter);

    m_details = AudioDeviceTypes::parse(portType(), beginIter, endIter);
  }
}

// properties
Byte AudioDeviceParm::versionNumber() const { return 0x01; }

const AudioDeviceTypes::USBDevice &AudioDeviceParm::usbDevice() const {
  assert(portType() == PortType::USBDevice);
  return boost::get<AudioDeviceTypes::USBDevice>(m_details);
}

AudioDeviceTypes::USBHost &AudioDeviceParm::usbHost() {
  assert(portType() == PortType::USBHost);
  return boost::get<AudioDeviceTypes::USBHost>(m_details);
}

const AudioDeviceTypes::USBHost &AudioDeviceParm::usbHost() const {
  assert(portType() == PortType::USBHost);
  return boost::get<AudioDeviceTypes::USBHost>(m_details);
}

}  // namespace GeneSysLib
