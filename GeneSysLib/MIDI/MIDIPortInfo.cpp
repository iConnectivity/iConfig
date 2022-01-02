/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MIDIPortInfo.h"
#include "Generator.h"

#include <limits>
#include <iterator>
#ifndef Q_MOC_RUN
#include <boost/range.hpp>
#endif

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t MIDIPortInfo::minKey() {
  return generateKey(Command::RetMIDIPortInfo);
}

commandDataKey_t MIDIPortInfo::maxKey() {
  return MIDIPortInfo::queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t MIDIPortInfo::queryKey(Word portID) {
  return generateKey(Command::RetMIDIPortInfo, portID);
}

CmdEnum MIDIPortInfo::retCommand() { return Command::RetMIDIInfo; }

CmdEnum MIDIPortInfo::setCommand() { return Command::SetMIDIInfo; }

MIDIPortInfo::MIDIPortInfo(void)
    : portID(), portType(), maxPortName(), portName(), m_portMIDIFlags() {
  memset(&m_portInfo, 0, sizeof(PortInfo));
}

const commandDataKey_t MIDIPortInfo::key() const {
  return generateKey(Command::RetMIDIPortInfo, portID());
}

Bytes MIDIPortInfo::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, portID());
  result += static_cast<Byte>(portType());
  copy(&m_portInfo.raw[0], &m_portInfo.raw[sizeof(PortInfo)],
       std::back_inserter(result));
  result += maxPortName();
  result += m_portMIDIFlags & 0x7F;
  copy(portName().begin(), portName().end(), std::back_inserter(result));

  return result;
}

void MIDIPortInfo::parse(BytesIter &begin, BytesIter &end) {
  auto version = nextMidiByte(begin, end);
  if (version == versionNumber()) {
    portID = nextROWord(begin, end);
    portType =
        roPortTypeEnum(static_cast<PortTypeEnum>(nextMidiByte(begin, end)));
#ifdef _WIN32
    copy(begin, begin + sizeof(PortInfo),
         stdext::checked_array_iterator<Byte *>(&m_portInfo.raw[0],
                                                sizeof(PortInfo)));
#else
    copy(begin, begin + sizeof(PortInfo), &m_portInfo.raw[0]);
#endif
    std::advance(begin, sizeof(PortInfo));
    maxPortName = nextROByte(begin, end);
    m_portMIDIFlags = nextMidiByte(begin, end);
    portName = rwString(string(begin, end));
  }
}

Byte MIDIPortInfo::versionNumber() const { return 0x01; }

MIDIPortInfo::PortInfo MIDIPortInfo::portInfo() const { return m_portInfo; }

bool MIDIPortInfo::isInputEnabled() const {
  return (m_portMIDIFlags & PortMIDIFlags::InputEnabled) ==
         PortMIDIFlags::InputEnabled;
}

void MIDIPortInfo::setInputEnabled(bool enabled) {
  m_portMIDIFlags &= ~PortMIDIFlags::InputEnabled;
  if (enabled) {
    m_portMIDIFlags |= PortMIDIFlags::InputEnabled;
  }
}

bool MIDIPortInfo::isOutputEnabled() const {
  return (m_portMIDIFlags & PortMIDIFlags::OutputEnabled) ==
         PortMIDIFlags::OutputEnabled;
}

void MIDIPortInfo::setOutputEnabled(bool enabled) {
  m_portMIDIFlags &= ~PortMIDIFlags::OutputEnabled;
  if (enabled) {
    m_portMIDIFlags |= PortMIDIFlags::OutputEnabled;
  }
}

bool MIDIPortInfo::isOfType(PortTypeEnum _portType) const {
  return (portType() == _portType);
}

bool isOfType(const MIDIPortInfo &midiPortInfo, PortTypeEnum portType) {
  return midiPortInfo.isOfType(portType);
}

}  // namespace GeneSysLib
