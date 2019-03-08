/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "AudioPortParmTypes.h"
#include "StreamHelpers.h"

namespace GeneSysLib {
namespace AudioPortParmTypes {

USBDevice USBDevice::parse(BytesIter& beginIter, const BytesIter& endIter) {
  USBDevice device;
  device.jack = nextROByte(beginIter, endIter);
  device.m_flags = std::bitset<8>(nextMidiByte(beginIter, endIter) & 0x7F);
  return device;
}

USBDevice::USBDevice() : jack(), m_flags() {}

// roByte jack;
bool USBDevice::supportsPCAudio() const { return m_flags[pcSupported]; }

bool USBDevice::isPCAudioEnabled() const { return m_flags[pcEnabled]; }

void USBDevice::setPCAudioEnabled(bool value) { m_flags[pcEnabled] = value; }

bool USBDevice::supportsIOSAudio() const { return m_flags[iOSSupported]; }

bool USBDevice::isIOSAudioEnabled() const { return m_flags[iOSEnabled]; }

void USBDevice::setIOSAudioEnabled(bool value) { m_flags[iOSEnabled] = value; }

Byte USBDevice::flags() const {
  return static_cast<Byte>(m_flags.to_ulong() & 0x7F);
}

////////////////////////////////////////////////////////////////////////////////

USBHost USBHost::parse(BytesIter& beginIter, const BytesIter& endIter) {
  USBHost host;
  host.jack = nextROByte(beginIter, endIter);
  host.deviceNumber = nextROByte(beginIter, endIter);
  return host;
}

USBHost::USBHost() : jack(), deviceNumber() {}

////////////////////////////////////////////////////////////////////////////////

Ethernet Ethernet::parse(BytesIter& beginIter, const BytesIter& endIter) {
  Ethernet ethernet;
  ethernet.jack = nextROByte(beginIter, endIter);
  ethernet.deviceNumber = nextROByte(beginIter, endIter);
  return ethernet;
}

Ethernet::Ethernet() : jack(), deviceNumber() {}

////////////////////////////////////////////////////////////////////////////////

Analogue Analogue::parse(BytesIter& beginIter, const BytesIter& endIter) {
  Analogue analogue;
  analogue.port = nextROByte(beginIter, endIter);
  return analogue;
}

Analogue::Analogue() : port() {}

////////////////////////////////////////////////////////////////////////////////

Bytes generateVisitor::operator()(const USBDevice& usbDevice) const {
  Bytes result;

  appendMidiByte(result, usbDevice.jack());
  appendMidiByte(result, usbDevice.flags());

  return result;
}

Bytes generateVisitor::operator()(const USBHost& usbHost) const {
  Bytes result;

  appendMidiByte(result, usbHost.jack());
  appendMidiByte(result, usbHost.deviceNumber());

  return result;
}

Bytes generateVisitor::operator()(const Ethernet& ethernet) const {
  Bytes result;

  appendMidiByte(result, ethernet.jack());
  appendMidiByte(result, ethernet.deviceNumber());

  return result;
}

Bytes generateVisitor::operator()(const Analogue& analogue) const {
  Bytes result;

  appendMidiByte(result, analogue.port());

  return result;
}

}  // namespace AudioPortParmTypes
}  // namespace GeneSysLib
