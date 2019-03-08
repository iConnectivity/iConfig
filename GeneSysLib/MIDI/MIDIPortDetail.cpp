/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "stdafx.h"
#include "MIDIPortDetail.h"
#include "Generator.h"

#include <iterator>
#include <limits>
#ifndef Q_MOC_RUN
#include <boost/range.hpp>
#endif

using namespace boost;

namespace GeneSysLib {

commandDataKey_t MIDIPortDetail::minKey() {
  return generateKey(Command::RetMIDIPortDetail);
}

commandDataKey_t MIDIPortDetail::maxKey() {
  return MIDIPortDetail::queryKey(std::numeric_limits<Word>::max());
}

commandDataKey_t MIDIPortDetail::queryKey(Word portID) {
  return generateKey(Command::RetMIDIPortDetail, portID);
}

CmdEnum MIDIPortDetail::retCommand() { return Command::RetMIDIPortDetail; }

CmdEnum MIDIPortDetail::setCommand() { return Command::SetMIDIPortDetail; }

MIDIPortDetail::MIDIPortDetail(void) : portID(), portType(), m_portDetails() {}

const commandDataKey_t MIDIPortDetail::key() const {
  return generateKey(Command::RetMIDIPortDetail, portID());
}

Bytes MIDIPortDetail::generate() const {
  Bytes result;

  result += versionNumber() & 0x7F;

  appendMidiWord(result, portID());

  result += static_cast<Byte>(portType());

  if (portType() != PortType::DIN) {
    Bytes details = boost::apply_visitor(MIDIPortDetailTypes::detailVisitor(),
                                         m_portDetails);
    copy(details.begin(), details.end(), std::back_inserter(result));
  }

  return result;
}

void MIDIPortDetail::parse(BytesIter &beginIter, BytesIter &endIter) {
  auto version = nextMidiByte(beginIter, endIter);

  if (version == versionNumber()) {
    portID = roWord(nextMidiWord(beginIter, endIter));
    portType = roPortTypeEnum(
        static_cast<PortTypeEnum>(nextMidiByte(beginIter, endIter)));

    if (portType() == PortType::USBDevice) {
      MIDIPortDetailTypes::USBDevice usbDevice;
      usbDevice.parse(beginIter, endIter);

      m_portDetails = usbDevice;
    } else if (portType() == PortType::USBHost) {
      MIDIPortDetailTypes::USBHost usbHost;
      usbHost.parse(beginIter, endIter);

      m_portDetails = usbHost;
    } else if (portType() == PortType::Ethernet) {
      MIDIPortDetailTypes::Ethernet eth;
      eth.parse(beginIter, endIter);

      m_portDetails = eth;
    }
  }
}

Byte MIDIPortDetail::versionNumber() const { return 0x01; }

MIDIPortDetailTypes::USBDevice &MIDIPortDetail::getUSBDevice() {
  return boost::get<MIDIPortDetailTypes::USBDevice>(m_portDetails);
}

const MIDIPortDetailTypes::USBDevice &MIDIPortDetail::getUSBDevice() const {
  return boost::get<MIDIPortDetailTypes::USBDevice>(m_portDetails);
}

MIDIPortDetailTypes::USBHost &MIDIPortDetail::getUSBHost() {
  return boost::get<MIDIPortDetailTypes::USBHost>(m_portDetails);
}

const MIDIPortDetailTypes::USBHost &MIDIPortDetail::getUSBHost() const {
  return boost::get<MIDIPortDetailTypes::USBHost>(m_portDetails);
}

MIDIPortDetailTypes::Ethernet &MIDIPortDetail::getEthernet() {
  return boost::get<MIDIPortDetailTypes::Ethernet>(m_portDetails);
}

const MIDIPortDetailTypes::Ethernet &MIDIPortDetail::getEthernet() const {
  return boost::get<MIDIPortDetailTypes::Ethernet>(m_portDetails);
}

}  // namespace GeneSysLib
