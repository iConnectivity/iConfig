/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MIDIPortRoute.h"
#include "Generator.h"

#ifndef Q_MOC_RUN
#include <boost/range.hpp>
#endif
#include <cassert>
#include <limits>

using namespace boost;

namespace GeneSysLib {

commandDataKey_t MIDIPortRoute::minKey() {
  return generateKey(Command::RetMIDIPortRoute);
}

commandDataKey_t MIDIPortRoute::maxKey() {
  return MIDIPortRoute::queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t MIDIPortRoute::queryKey(Word portID) {
  return generateKey(Command::RetMIDIPortRoute, portID);
}

CmdEnum MIDIPortRoute::retCommand() { return Command::RetMIDIPortRoute; }

CmdEnum MIDIPortRoute::setCommand() { return Command::SetMIDIPortRoute; }

MIDIPortRoute::MIDIPortRoute(void) : portID(), m_portRouting() {}

MIDIPortRoute::MIDIPortRoute(Word _portID, Bytes portRouting)
    : portID(_portID), m_portRouting(portRouting) {}

const commandDataKey_t MIDIPortRoute::key() const {
  return generateKey(Command::RetMIDIPortRoute, portID());
}

Bytes MIDIPortRoute::generate() const {
  Bytes result;

  result += versionNumber();
  appendMidiWord(result, portID());
  copy(m_portRouting.begin(), m_portRouting.end(), std::back_inserter(result));

  return result;
}

void MIDIPortRoute::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    portID = nextROWord(beginIter, endIter);
    m_portRouting.clear();
    copy(beginIter, endIter, std::back_inserter(m_portRouting));
  }
}

Byte MIDIPortRoute::versionNumber() const { return 0x01; }

bool MIDIPortRoute::isRoutedTo(Byte destPortID) const {
  auto bitOff = 1 << ((destPortID - 1) & 0x03);
  auto byteOff = ((destPortID - 1) / 4);
  assert(static_cast<size_t>(byteOff) < m_portRouting.size());
  return (m_portRouting[byteOff] & bitOff) == bitOff;
}

void MIDIPortRoute::setRoutedTo(Byte destPortID, bool value) {
  auto bitOff = 1 << ((destPortID - 1) & 0x03);
  auto byteOff = ((destPortID - 1) / 4);
  assert(static_cast<size_t>(byteOff) < m_portRouting.size());
  m_portRouting[byteOff] &= ~bitOff;
  if (value) {
    m_portRouting[byteOff] |= bitOff;
  }
}

}  // namespace GeneSysLib
