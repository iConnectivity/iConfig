/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "Device.h"
#include "Generator.h"

namespace GeneSysLib {

CmdEnum Device::retCommand() { return Command::RetDevice; }

Device::Device(void) : m_protocol(0), m_mode(0), m_maxLength(0) {}

Device::Device(Byte protocol, Byte mode, Word maxLength)
    : m_protocol(protocol), m_mode(mode), m_maxLength(maxLength) {}

// overloaded methods
const commandDataKey_t Device::key() const {
  return generateKey(Command::RetDevice);
}

Bytes Device::generate() const {
  Bytes result;
  result += (m_protocol & 0x7F), (m_mode & 0x7F);
  appendMidiWord(result, m_maxLength);
  return result;
}

void Device::parse(BytesIter &begin, BytesIter &end) {
  m_protocol = nextMidiByte(begin, end);
  m_mode = nextMidiByte(begin, end);
  m_maxLength = nextMidiWord(begin, end);
}

Byte Device::protocol() const { return m_protocol; }

Byte Device::mode() const { return m_mode; }

Word Device::maxLength() const { return m_maxLength; }

}  // namespace GeneSysLib
