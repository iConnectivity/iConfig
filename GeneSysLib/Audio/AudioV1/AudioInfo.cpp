/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "AudioInfo.h"
#include "Generator.h"

namespace GeneSysLib {

CmdEnum AudioInfo::retCommand() { return Command::RetAudioInfo; }

AudioInfo::AudioInfo(void)
    : numberOfAudioPorts(0),
      numberOfUSBDeviceJacks(0),
      numberOfUSBHostJacks(0),
      numberOfEthernetJacks(0),
      numberOfPortsPerUSBHostJack(0),
      numberOfPortsPerEthernetJack(0) {}

const commandDataKey_t AudioInfo::key() const {
  return generateKey(Command::RetAudioInfo);
}

Bytes AudioInfo::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, numberOfAudioPorts());
  result += numberOfUSBDeviceJacks();
  result += numberOfUSBHostJacks();
  result += numberOfPortsPerUSBHostJack();
  result += numberOfPortsPerEthernetJack();

  return result;
}

void AudioInfo::parse(BytesIter &begin, BytesIter &end) {
  const auto &version = nextMidiByte(begin, end);
  // only handle version 1
  if (version == versionNumber()) {
    numberOfAudioPorts = roWord(nextMidiWord(begin, end));
    numberOfUSBDeviceJacks = roByte(nextMidiByte(begin, end));
    numberOfUSBHostJacks = roByte(nextMidiByte(begin, end));
    numberOfPortsPerUSBHostJack = roByte(nextMidiByte(begin, end));
    numberOfPortsPerEthernetJack = roByte(nextMidiByte(begin, end));
  }
}

Byte AudioInfo::versionNumber() const { return 0x01; }

}  // namespace GeneSysLib
