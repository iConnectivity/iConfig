/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef MIDIPORTDETAILTYPES_H
#define MIDIPORTDETAILTYPES_H

#include "LibTypes.h"
#include "PortType.h"
#include "HostType.h"

#ifndef Q_MOC_RUN
#include <boost/variant.hpp>
#endif

namespace GeneSysLib {

struct USBHostMIDIDeviceDetail;

namespace MIDIPortDetailTypes {

struct USBDevice {
  void parse(BytesIter &beginIter, BytesIter &endIter);

  HostType::Enum hostType() const;
  const std::string &hostName() const;

 private:
  HostType::Enum m_hostType;
  std::string m_hostName;
};  // struct USBDevice

////////////////////////////////////////////////////////////////////////////////

struct USBHost {
  void parse(BytesIter &beginIter, BytesIter &endIter);

  Byte flags() const;
  Byte usbHostID() const;
  void usbHostID(Byte hostID);
  Byte hostedDeviceMIDIPort() const;
  void hostedDeviceMIDIPort(Byte port);
  Word hostedUSBVendorID() const;
  void hostedUSBVendorID(Word vendorID);
  Word hostedUSBProductID() const;
  void hostedUSBProductID(Word productID);
  std::string vendorName() const;
  void vendorName(std::string vendorName);
  std::string productName() const;
  void productName(std::string productName);
  void notReserved();
  bool isReserved();
  void reserved(const USBHostMIDIDeviceDetail &hostDeviceData, int hostPortID);

 private:
  Byte m_flags;
  Byte m_usbHostID;
  Byte m_hostedDevicesMIDIPort;
  Word m_hostedUSBVendorID;
  Word m_hostedUSBProductID;
  std::string m_vendorName;
  std::string m_productName;
};  // struct USBHost

////////////////////////////////////////////////////////////////////////////////

struct Ethernet {
  void parse(BytesIter &beginIter, BytesIter &endIter);

  Word rtpMIDIPortNumber() const;
  Byte numActiveRTPConnections() const;
  const std::string &sessionName() const;

 private:
  Word m_rtpMIDIPortNumber;
  Byte m_numActiveRTPConnections;
  std::string m_sessionName;
};  // struct Ethernet

////////////////////////////////////////////////////////////////////////////////

typedef boost::variant<USBDevice, USBHost, Ethernet> Variants;

struct detailVisitor : public boost::static_visitor<Bytes> {
  Bytes operator()(const USBDevice &usbDevice) const;
  Bytes operator()(const USBHost &usbHost) const;
  Bytes operator()(const Ethernet &ethernet) const;
};  // struct detailVisitor

}  // namespace MIDIPortDetailTypes

}  // namespace GeneSysLib

#endif  // MIDIPORTDETAILTYPES_H
