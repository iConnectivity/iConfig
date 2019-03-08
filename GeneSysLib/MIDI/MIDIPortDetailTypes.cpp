/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MIDIPortDetailTypes.h"
#include "USBHostMIDIDeviceDetail.h"
#include "StreamHelpers.h"

using namespace std;

namespace GeneSysLib {

namespace MIDIPortDetailTypes {

void USBDevice::parse(BytesIter &beginIter, BytesIter &endIter) {
  m_hostType = (HostType::Enum)nextMidiByte(beginIter, endIter);
  Byte length = nextMidiByte(beginIter, endIter);
  if (length > 0) {
    m_hostName = string(beginIter, (beginIter + length));
    advance(beginIter, length);
  }
}
HostType::Enum USBDevice::hostType() const { return m_hostType; }

const std::string &USBDevice::hostName() const { return m_hostName; }

////////////////////////////////////////////////////////////////////////////////

Byte USBHost::flags() const { return m_flags; }

Byte USBHost::usbHostID() const { return m_usbHostID; }

void USBHost::usbHostID(Byte hostID) { m_usbHostID = hostID; }

Byte USBHost::hostedDeviceMIDIPort() const { return m_hostedDevicesMIDIPort; }

void USBHost::hostedDeviceMIDIPort(Byte port) {
  m_hostedDevicesMIDIPort = port;
}

Word USBHost::hostedUSBVendorID() const { return m_hostedUSBVendorID; }

void USBHost::hostedUSBVendorID(Word vendorID) {
  m_hostedUSBVendorID = vendorID;
}

Word USBHost::hostedUSBProductID() const { return m_hostedUSBProductID; }

void USBHost::hostedUSBProductID(Word productID) {
  m_hostedUSBProductID = productID;
}

std::string USBHost::vendorName() const { return m_vendorName; }

void USBHost::vendorName(std::string vendorName) { m_vendorName = vendorName; }

std::string USBHost::productName() const { return m_productName; }

void USBHost::productName(std::string productName) {
  m_productName = productName;
}

void USBHost::notReserved() {
  m_flags = 0x00;
  m_usbHostID = 0x0000;
  m_hostedDevicesMIDIPort = 0x0000;
  m_hostedUSBVendorID = 0x0000;
  m_hostedUSBProductID = 0x0000;
  m_productName.clear();
  m_vendorName.clear();
}

bool USBHost::isReserved() { return (m_flags & 0x01) == 0x01; }

void USBHost::reserved(const USBHostMIDIDeviceDetail &hostDeviceData,
                       int hostPortID) {
  m_flags = 0x01;  // reserved -> TODO: remove magic numbers
  m_usbHostID = hostDeviceData.usbHostID();
  m_hostedDevicesMIDIPort = hostPortID;
  m_hostedUSBVendorID = hostDeviceData.hostedUSBVendorID();
  m_hostedUSBProductID = hostDeviceData.hostedUSBProductID();
  m_vendorName = hostDeviceData.vendorName();
  m_productName = hostDeviceData.productName();
}

void USBHost::parse(BytesIter &beginIter, BytesIter &endIter) {
  m_flags = nextMidiByte(beginIter, endIter);
  m_usbHostID = nextMidiByte(beginIter, endIter);
  m_hostedDevicesMIDIPort = nextMidiByte(beginIter, endIter);

  m_hostedUSBVendorID = nextMidiByte(beginIter, endIter);
  m_hostedUSBVendorID =
      ((m_hostedUSBVendorID << 7) | (nextMidiByte(beginIter, endIter)));
  m_hostedUSBVendorID =
      ((m_hostedUSBVendorID << 7) | (nextMidiByte(beginIter, endIter)));

  m_hostedUSBProductID = nextMidiByte(beginIter, endIter);
  m_hostedUSBProductID =
      ((m_hostedUSBProductID << 7) | (nextMidiByte(beginIter, endIter)));
  m_hostedUSBProductID =
      ((m_hostedUSBProductID << 7) | (nextMidiByte(beginIter, endIter)));

  Byte length = nextMidiByte(beginIter, endIter);

  if (length > 0) {
    m_vendorName = string(beginIter, (beginIter + length));
    advance(beginIter, length);
  }

  length = nextMidiByte(beginIter, endIter);
  if (length > 0) {
    m_productName = string(beginIter, (beginIter + length));
    advance(beginIter, length);
  }
}

////////////////////////////////////////////////////////////////////////////////

void Ethernet::parse(BytesIter &beginIter, BytesIter &endIter) {
  m_rtpMIDIPortNumber = fromMidiWordIn3Bytes(beginIter, endIter);
  m_numActiveRTPConnections = nextMidiByte(beginIter, endIter);
  Byte length = nextMidiByte(beginIter, endIter);
  if (length > 0) {
    m_sessionName = string(beginIter, (beginIter + length));
  }
}

Word Ethernet::rtpMIDIPortNumber() const { return m_rtpMIDIPortNumber; }

Byte Ethernet::numActiveRTPConnections() const {
  return m_numActiveRTPConnections;
}

const std::string &Ethernet::sessionName() const { return m_sessionName; }

Bytes detailVisitor::operator()(const USBDevice &usbDevice) const {
  Bytes result;
  result += (Byte)usbDevice.hostType();
  result += (Byte)(usbDevice.hostName().size() & 0x7F);
  const auto &hostName = usbDevice.hostName();
  copy(hostName.begin(), hostName.end(), std::back_inserter(result));

  return result;
}

Bytes detailVisitor::operator()(const USBHost &usbHost) const {
  Bytes result;
  result += (Byte)usbHost.flags();
  result += (Byte)usbHost.usbHostID();
  result += (Byte)usbHost.hostedDeviceMIDIPort();

  // Add Hosted USB Vendor ID
  appendMidiWord3Byte(result, usbHost.hostedUSBVendorID());

  // Add Hosted USB Product ID
  appendMidiWord3Byte(result, usbHost.hostedUSBProductID());

  // Add Vendor Name
  const auto &vendorName = usbHost.vendorName();
  result += (Byte)(vendorName.size() & 0x7F);
  copy(vendorName.begin(), vendorName.end(), std::back_inserter(result));

  // Add Product Name
  const auto &productName = usbHost.productName();
  result += (Byte)(productName.size() & 0x7F);
  copy(productName.begin(), productName.end(), std::back_inserter(result));

  return result;
}

Bytes detailVisitor::operator()(const Ethernet &ethernet) const {
  Bytes result;
  appendMidiWord(result, ethernet.rtpMIDIPortNumber());
  result += (Byte)ethernet.numActiveRTPConnections();

  const auto &sessionName = ethernet.sessionName();
  result += (Byte)(sessionName.size() & 0x7F);
  copy(sessionName.begin(), sessionName.end(), std::back_inserter(result));

  return result;
}

}  // namespace PortDetails

}  // namespace GeneSysLib
