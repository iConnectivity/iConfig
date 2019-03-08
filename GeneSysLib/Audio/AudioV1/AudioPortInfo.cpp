/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "AudioPortInfo.h"
#include "Generator.h"
#include <limits>
#ifndef Q_MOC_RUN
#include <boost/range.hpp>
#endif

using namespace boost;
using namespace std;

namespace GeneSysLib {

commandDataKey_t AudioPortInfo::minKey() {
  return generateKey(Command::RetAudioPortInfo);
}

commandDataKey_t AudioPortInfo::maxKey() {
  return AudioPortInfo::queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t AudioPortInfo::queryKey(Word portID) {
  return generateKey(Command::RetAudioPortInfo, portID);
}

CmdEnum AudioPortInfo::retCommand() { return Command::RetAudioPortInfo; }

CmdEnum AudioPortInfo::setCommand() { return Command::SetAudioPortInfo; }

AudioPortInfo::AudioPortInfo(void)
    : portID(),
      portType(),
      maxPortName(),
      portName(),
      m_portInfo(),
      m_portSpecificOptions() {}

const commandDataKey_t AudioPortInfo::key() const {
  return generateKey(Command::RetAudioPortInfo, portID());
}

Bytes AudioPortInfo::generate() const {
  Bytes result;

  result += static_cast<Byte>(versionNumber());
  appendMidiWord(result, portID());
  result += static_cast<Byte>(portType());
  copy(m_portInfo.raw, m_portInfo.raw + sizeof(PortInfo),
       back_inserter(result));
  result += maxPortName();

  unsigned long val = m_portSpecificOptions.to_ulong();
  result += static_cast<Byte>((val >> 24) & 0x7F);
  result += static_cast<Byte>((val >> 16) & 0x7F);
  result += static_cast<Byte>((val >> 8) & 0x7F);
  result += static_cast<Byte>(val & 0x7F);
  copy(portName().begin(), portName().end(), back_inserter(result));

  return result;
}

void AudioPortInfo::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);
  if (version == versionNumber()) {
    portID = roWord(nextMidiWord(beginIter, endIter));
    portType = roPortTypeEnum(
        static_cast<PortTypeEnum>(nextMidiByte(beginIter, endIter)));
#ifdef _WIN32
    copy(beginIter, beginIter + sizeof(PortInfo),
         stdext::checked_array_iterator<Byte *>(&m_portInfo.raw[0],
                                                sizeof(PortInfo)));
#else
    copy(beginIter, beginIter + sizeof(PortInfo), &m_portInfo.raw[0]);
#endif
    advance(beginIter, sizeof(PortInfo));

    maxPortName = roByte(nextMidiByte(beginIter, endIter));

    long val;
    val = ((nextMidiByte(beginIter, endIter)) << 8);
    val = (val << 8) | (nextMidiByte(beginIter, endIter));
    val = (val << 8) | (nextMidiByte(beginIter, endIter));
    val = (val << 8) | (nextMidiByte(beginIter, endIter));

    m_portSpecificOptions = PortSpecificOptions(val);

    portName = rwString(string(beginIter, endIter));
  }
}

Byte AudioPortInfo::versionNumber() const { return 0x01; }

AudioPortInfo::PortInfo AudioPortInfo::portInfo() const { return m_portInfo; }

bool AudioPortInfo::canEditPortName() const { return (maxPortName() > 0); }

bool AudioPortInfo::supportsPCAudio() const {
  return m_portSpecificOptions[PortSpecificOptionsBit::portSupportsPCAudio];
}

bool AudioPortInfo::isPCAudioEnabled() const {
  return m_portSpecificOptions[PortSpecificOptionsBit::enabledAudioForPC];
}

void AudioPortInfo::setPCAudioEnabled(bool isSet) {
  m_portSpecificOptions.set(PortSpecificOptionsBit::enabledAudioForPC, isSet);
}

bool AudioPortInfo::supportsIOSAudio() const {
  return m_portSpecificOptions[PortSpecificOptionsBit::portSupportsiOSAudio];
}

bool AudioPortInfo::isIOSAudioEnabled() const {
  return m_portSpecificOptions[PortSpecificOptionsBit::enabledAudioForiOS];
}

void AudioPortInfo::setIOSAudioEnabled(bool isSet) {
  m_portSpecificOptions.set(PortSpecificOptionsBit::enabledAudioForiOS, isSet);
}

}  // namespace GeneSysLib
