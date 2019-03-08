/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioPortParm.h"
#include "CommandDefines.h"
#include "Generator.h"
#include "StreamHelpers.h"
#include <limits>

using namespace std;

namespace GeneSysLib {

#define kNoConfigBlocks (0x00)

commandDataKey_t AudioPortParm::minKey() {
  return generateKey(Command::RetAudioPortParm);
}

commandDataKey_t AudioPortParm::maxKey() {
  return queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t AudioPortParm::queryKey(Word portID) {
  return generateKey(AudioPortParm::retCommand(), portID);
}

CmdEnum AudioPortParm::retCommand() { return Command::RetAudioPortParm; }

CmdEnum AudioPortParm::setCommand() { return Command::SetAudioPortParm; }

////////////////////////////////////////////////////////////////////////////////

AudioPortParm::ConfigBlock::ConfigBlock(void)
    : audioConfigNumber(),
      maxAudioChannels(),
      minInputChannels(),
      maxInputChannels(),
      minOutputChannels(),
      maxOutputChannels() {}

void AudioPortParm::ConfigBlock::parse(BytesIter &beginIter,
                                       BytesIter &endIter) {
  audioConfigNumber = nextROByte(beginIter, endIter);
  maxAudioChannels = nextROByte(beginIter, endIter);
  minInputChannels = nextROByte(beginIter, endIter);
  maxInputChannels = nextROByte(beginIter, endIter);
  minOutputChannels = nextROByte(beginIter, endIter);
  maxOutputChannels = nextROByte(beginIter, endIter);
}

AudioPortParm::ConfigBlock AudioPortParm::ConfigBlock::parseConfigBlock(
    BytesIter &beginIter, BytesIter &endIter) {
  ConfigBlock block;
  block.parse(beginIter, endIter);
  return block;
}

bool AudioPortParm::ConfigBlock::isInputSelectable() const {
  return minInputChannels() < maxInputChannels();
}

bool AudioPortParm::ConfigBlock::isOutputSelectable() const {
  return minOutputChannels() < maxOutputChannels();
}

////////////////////////////////////////////////////////////////////////////////

AudioPortParm::AudioPortParm(void)
    : audioPortID(),
      portType(),
      numInputChannels(),
      numOutputChannels(),
      maxPortName(),
      portName(),
      m_configBlocks(),
      m_details() {}

// overloaded methods
const commandDataKey_t AudioPortParm::key() const {
  return queryKey(audioPortID());
}

Bytes AudioPortParm::generate() const {
  Bytes result;

  appendMidiByte(result, versionNumber());
  appendMidiWord(result, audioPortID());
  appendMidiByte(result, static_cast<Byte>(portType()));
  appendMidiByte(result, numInputChannels());
  appendMidiByte(result, numOutputChannels());
  appendMidiByte(result, kNoConfigBlocks);
  appendMidiByte(result, maxPortName());
  appendMidiByte(result, portName().size());
  appendString(result, portName());

  appendBytes(result, AudioPortParmTypes::generate(m_details));

  return result;
}

void AudioPortParm::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    audioPortID = nextROWord(beginIter, endIter);
    portType = roPortTypeEnum(
        static_cast<PortTypeEnum>(nextMidiByte(beginIter, endIter)));
    numInputChannels = nextRWByte(beginIter, endIter);
    numOutputChannels = nextRWByte(beginIter, endIter);
    auto tmpNumConfigBlocks = nextMidiByte(beginIter, endIter);
    m_configBlocks.clear();
    for (int i = 0; i < tmpNumConfigBlocks; ++i) {
      m_configBlocks.push_back(
          ConfigBlock::parseConfigBlock(beginIter, endIter));
    }
    maxPortName = nextROByte(beginIter, endIter);
    auto tmpNameLength = nextMidiByte(beginIter, endIter);
    portName = nextRWString(tmpNameLength, beginIter, endIter);

    m_details = AudioPortParmTypes::parse(portType(), beginIter, endIter);
  }
}

// properties
Byte AudioPortParm::versionNumber() const { return 0x01; }

Byte AudioPortParm::jack() const {
  Byte result = 0x00;

  if (portType() == PortType::USBDevice) {
    result = usbDevice().jack();
  } else if (portType() == PortType::USBHost) {
    result = usbHost().jack();
  } else if (portType() == PortType::Ethernet) {
    result = ethernet().jack();
  }

  return result;
}

Byte AudioPortParm::numConfigBlocks() const { return m_configBlocks.size(); }

const AudioPortParm::ConfigBlock &AudioPortParm::block_at(
    Byte audioConfigNumber) const {
  assert(audioConfigNumber > 0);
  assert(audioConfigNumber <= m_configBlocks.size());
  return m_configBlocks.at(audioConfigNumber - 1);
}

bool AudioPortParm::canEditPortName() const { return maxPortName() > 0; }

bool AudioPortParm::isOfType(PortTypeEnum _portType) const {
  return (portType() == _portType);
}

AudioPortParmTypes::USBDevice &AudioPortParm::usbDevice() {
  assert(isOfType(PortType::USBDevice));
  return boost::get<AudioPortParmTypes::USBDevice>(m_details);
}

const AudioPortParmTypes::USBDevice &AudioPortParm::usbDevice() const {
  assert(isOfType(PortType::USBDevice));
  return boost::get<AudioPortParmTypes::USBDevice>(m_details);
}

const AudioPortParmTypes::USBHost &AudioPortParm::usbHost() const {
  assert(isOfType(PortType::USBHost));
  return boost::get<AudioPortParmTypes::USBHost>(m_details);
}

const AudioPortParmTypes::Ethernet &AudioPortParm::ethernet() const {
  assert(isOfType(PortType::Ethernet));
  return boost::get<AudioPortParmTypes::Ethernet>(m_details);
}

const AudioPortParmTypes::Analogue &AudioPortParm::analogue() const {
  assert(isOfType(PortType::Analogue));
  return boost::get<AudioPortParmTypes::Analogue>(m_details);
}

}  // namespace GeneSysLib
