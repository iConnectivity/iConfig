/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MIDIInfo.h"
#include "Generator.h"

namespace GeneSysLib {

CmdEnum MIDIInfo::retCommand() { return Command::RetMIDIInfo; }
CmdEnum MIDIInfo::setCommand() { return Command::SetMIDIInfo; }

MIDIInfo::MIDIInfo(void)
    : numMIDIPorts(),
      hostMIDIPort(),
      numDINPairs(),
      numUSBDeviceJacks(),
      numUSBHostJacks(),
      numEthernetJacks(),
      numUSBMIDIPortPerDeviceJack(),
      numUSBMIDIPortPerHostJack(),
      numRTPMIDISessionsPerEthernetJack(),
      numRTPMIDIConnectionsPerSession(),
      maxPortsOnMultiPortUSBDevice(),
      m_globalMIDIFlags() {}

MIDIInfo::MIDIInfo(Byte _globalMIDIFlags, Byte _maxPortsOnMultiPortUSBDevice)
    : numMIDIPorts(),
      hostMIDIPort(),
      numDINPairs(),
      numUSBDeviceJacks(),
      numUSBHostJacks(),
      numEthernetJacks(),
      numUSBMIDIPortPerDeviceJack(),
      numUSBMIDIPortPerHostJack(),
      numRTPMIDISessionsPerEthernetJack(),
      numRTPMIDIConnectionsPerSession(),
      maxPortsOnMultiPortUSBDevice(_maxPortsOnMultiPortUSBDevice),
      m_globalMIDIFlags(_globalMIDIFlags) {}

MIDIInfo::MIDIInfo(Word _numMIDIPorts, Word _hostMIDIPort, Byte _numDINPairs,
                   Byte _numUSBDeviceJacks, Byte _numUSBHostJacks,
                   Byte _numEthernetJacks, Byte _numUSBMIDIPortPerDeviceJack,
                   Byte _numUSBMIDIPortPerHostJack,
                   Byte _numRTPMIDISessionsPerEthernetJack,
                   Byte _numRTPMIDIConnectionsPerEthernetJack,
                   Byte _maxPortsOnMultiPortUSBDevice, Byte _globalMIDIFlags)
    : numMIDIPorts(_numMIDIPorts),
      hostMIDIPort(_hostMIDIPort),
      numDINPairs(_numDINPairs),
      numUSBDeviceJacks(_numUSBDeviceJacks),
      numUSBHostJacks(_numUSBHostJacks),
      numEthernetJacks(_numEthernetJacks),
      numUSBMIDIPortPerDeviceJack(_numUSBMIDIPortPerDeviceJack),
      numUSBMIDIPortPerHostJack(_numUSBMIDIPortPerHostJack),
      numRTPMIDISessionsPerEthernetJack(_numRTPMIDISessionsPerEthernetJack),
      numRTPMIDIConnectionsPerSession(_numRTPMIDIConnectionsPerEthernetJack),
      maxPortsOnMultiPortUSBDevice(_maxPortsOnMultiPortUSBDevice),
      m_globalMIDIFlags(_globalMIDIFlags) {}

const commandDataKey_t MIDIInfo::key() const {
  return generateKey(Command::RetMIDIInfo);
}

Bytes MIDIInfo::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, numMIDIPorts());
  appendMidiWord(result, hostMIDIPort());
  result += numDINPairs();
  result += numUSBDeviceJacks();
  result += numUSBHostJacks();
  result += numEthernetJacks();
  result += numUSBMIDIPortPerDeviceJack();
  result += numUSBMIDIPortPerHostJack();
  result += numRTPMIDISessionsPerEthernetJack();
  result += numRTPMIDIConnectionsPerSession();
  result += m_globalMIDIFlags;
  result += maxPortsOnMultiPortUSBDevice();

  return result;
}

void MIDIInfo::parse(BytesIter &begin, BytesIter &end) {
  auto version = nextMidiByte(begin, end);
  if (version == versionNumber()) {
    numMIDIPorts = roWord(nextMidiWord(begin, end));
    hostMIDIPort = roWord(nextMidiWord(begin, end));
    numDINPairs = roByte(nextMidiByte(begin, end));
    numUSBDeviceJacks = roByte(nextMidiByte(begin, end));
    numUSBHostJacks = roByte(nextMidiByte(begin, end));
    numEthernetJacks = roByte(nextMidiByte(begin, end));
    numUSBMIDIPortPerDeviceJack = roByte(nextMidiByte(begin, end));
    numUSBMIDIPortPerHostJack = roByte(nextMidiByte(begin, end));
    numRTPMIDISessionsPerEthernetJack = roByte(nextMidiByte(begin, end));
    numRTPMIDIConnectionsPerSession = roByte(nextMidiByte(begin, end));
    m_globalMIDIFlags = nextMidiByte(begin, end);
    maxPortsOnMultiPortUSBDevice = rwByte(nextMidiByte(begin, end));
  }
}

Byte MIDIInfo::versionNumber() const { return 0x01; }

Byte MIDIInfo::globalMIDIFlags() const { return m_globalMIDIFlags; }

bool MIDIInfo::isSet(GlobalMIDIFlags::Enum globalFlag) const {
  return (m_globalMIDIFlags & globalFlag) == globalFlag;
}

void MIDIInfo::setFlag(GlobalMIDIFlags::Enum globalFlag, bool value) {
  m_globalMIDIFlags &= ~globalFlag;
  if (value) {
    m_globalMIDIFlags |= globalFlag;
  }
}

}  // namespace GeneSysLib
