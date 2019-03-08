/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MIDIPortRemap.h"
#include "Generator.h"

#include <limits>

using namespace boost::assign;
using namespace boost::range;

namespace GeneSysLib {

commandDataKey_t MIDIPortRemap::minKey() {
  return generateKey(Command::RetMIDIPortRemap);
}

commandDataKey_t MIDIPortRemap::maxKey() {
  return MIDIPortRemap::queryKey(std::numeric_limits<Word>::max(),
                                 RemapID::Unknown);
}

commandDataKey_t MIDIPortRemap::queryKey(Word portID, RemapTypeEnum remapType) {
  return generateKey(Command::RetMIDIPortRemap, portID, remapType);
}

CmdEnum MIDIPortRemap::retCommand() { return Command::RetMIDIPortRemap; }

CmdEnum MIDIPortRemap::setCommand() { return Command::SetMIDIPortRemap; }

Bytes MIDIPortRemap::RemapStatus::generate() const {
  Bytes result;
  Byte firstByte = 0x00;
  if (pitchBendEvents) {
    firstByte |= 0x20;
  }
  if (channelPressureEvents) {
    firstByte |= 0x10;
  }
  if (programChangeEvents) {
    firstByte |= 0x08;
  }
  if (controlChangeEvents) {
    firstByte |= 0x04;
  }
  if (polyKeyPressureEvents) {
    firstByte |= 0x02;
  }
  if (noteEvents) {
    firstByte |= 0x01;
  }

  result += firstByte;
  result += channelNumber & 0x0F;
  return result;
}

void MIDIPortRemap::RemapStatus::parse(BytesIter &beginIter,
                                       BytesIter &endIter) {
  Byte byte = nextMidiByte(beginIter, endIter);
  pitchBendEvents = ((byte & 0x20) == 0x20);
  channelPressureEvents = ((byte & 0x10) == 0x10);
  programChangeEvents = ((byte & 0x08) == 0x08);
  controlChangeEvents = ((byte & 0x04) == 0x04);
  polyKeyPressureEvents = ((byte & 0x02) == 0x02);
  noteEvents = ((byte & 0x01) == 0x01);
  channelNumber = (nextMidiByte(beginIter, endIter)) & 0x0F;
}

Bytes MIDIPortRemap::RemapFlags::generate() const {
  Bytes result;
  auto temp = channelBitmap.to_ulong();
  result += (Byte)((temp >> 24) & 0x7F);
  result += (Byte)((temp >> 16) & 0x7F);
  result += (Byte)((temp >> 8) & 0x7F);
  result += (Byte)(temp & 0x7F);

  result += controllerSource & 0x7F;
  result += controllerDestination & 0x7F;
  return result;
}

void MIDIPortRemap::RemapFlags::parse(BytesIter &beginIter,
                                      BytesIter &endIter) {
  long temp = 0x00;

  temp = nextMidiByte(beginIter, endIter);
  temp = (temp << 8) | (nextMidiByte(beginIter, endIter) & 0x7FL);
  temp = (temp << 8) | (nextMidiByte(beginIter, endIter) & 0x7FL);
  temp = (temp << 8) | (nextMidiByte(beginIter, endIter) & 0x7FL);

  channelBitmap = ChannelBitmap(temp);

  controllerSource = nextMidiByte(beginIter, endIter);
  controllerDestination = nextMidiByte(beginIter, endIter);
}

MIDIPortRemap::MIDIPortRemap(void)
    : portID(),
      remapID(),
      maxControllerSupported(),
      m_remapStatuses(),
      m_controllers() {}

MIDIPortRemap::MIDIPortRemap(Word _portID, RemapTypeEnum _remapID,
                             Byte _maxControllerSupported,
                             RemapStatues remapStatuses,
                             RemapFlagsVector controllers)
    : portID(_portID),
      remapID(_remapID),
      maxControllerSupported(_maxControllerSupported),
      m_remapStatuses(remapStatuses),
      m_controllers(controllers) {}

const commandDataKey_t MIDIPortRemap::key() const {
  return generateKey(Command::RetMIDIPortRemap, portID(), remapID());
}

Bytes MIDIPortRemap::generate() const {
  Bytes result;

  result += versionNumber() & 0x7F;
  appendMidiWord(result, portID());
  result += static_cast<Byte>(remapID());
  result += maxControllerSupported();

  for (const auto &rs : m_remapStatuses) {
    push_back(result, rs.generate());
  }

  for (const auto &ctrl : m_controllers) {
    push_back(result, ctrl.generate());
  }

  return result;
}

void MIDIPortRemap::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    portID = nextROWord(beginIter, endIter);
    remapID = roRemapTypeEnum(
        static_cast<RemapTypeEnum>(nextMidiByte(beginIter, endIter)));
    maxControllerSupported = nextROByte(beginIter, endIter);

    for (auto &rs : m_remapStatuses) {
      rs.parse(beginIter, endIter);
    }

    m_controllers.clear();
    for (auto i = 0; i < maxControllerSupported(); ++i) {
      RemapFlags rf;
      rf.parse(beginIter, endIter);
      m_controllers.push_back(rf);
    }
  }
}

Byte MIDIPortRemap::versionNumber() const { return 0x01; }

size_t MIDIPortRemap::numRemapStatuses() const {
  return m_remapStatuses.size();
}

MIDIPortRemap::RemapStatus &MIDIPortRemap::remapStatus_at(size_t index) {
  return m_remapStatuses.at(index);
}

const MIDIPortRemap::RemapStatus &MIDIPortRemap::remapStatus_at(
    size_t index) const {
  return m_remapStatuses.at(index);
}

size_t MIDIPortRemap::numControllers() const { return m_controllers.size(); }

MIDIPortRemap::RemapFlags &MIDIPortRemap::controller_at(size_t index) {
  return m_controllers.at(index);
}

const MIDIPortRemap::RemapFlags &MIDIPortRemap::controller_at(
    size_t index) const {
  return m_controllers.at(index);
}

}  // namespace GeneSysLib
