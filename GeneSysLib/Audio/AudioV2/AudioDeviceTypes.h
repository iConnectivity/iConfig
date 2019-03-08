/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIODEVICETYPES_H__
#define __AUDIODEVICETYPES_H__

#include "property.h"
#include "LibTypes.h"
#include "HostType.h"
#include "PortType.h"
#include <bitset>
#include <vector>
#include <map>
#ifndef Q_MOC_RUN
#include <boost/variant.hpp>
#endif

namespace GeneSysLib {

namespace AudioDeviceTypes {

struct USBDevice {
  static USBDevice parse(BytesIter &beginIter, const BytesIter &endIter);
  roHostTypeEnum hostType;
  roString hostName;
};

struct USBHost {
  enum Flags {
    connectedFlag = 6,  // bit 6
    isReservedFlag = 0  // bit 0
  };

  static USBHost parse(BytesIter &beginIter, const BytesIter &endIter);

  bool connected() const;
  bool reserved() const;
  void reserved(bool value);

  roByte maxInChannels;
  roByte maxOutChannels;
  std::map<Byte, Byte> inChannelMap;
  std::map<Byte, Byte> outChannelMap;
  roWord vendorID;
  roWord productID;
  roString vendorName;
  roString productName;

  std::bitset<8> flags;
};

struct Ethernet {
  static Ethernet parse(BytesIter &, const BytesIter &);
};

struct Analogue {
  static Analogue parse(BytesIter &, const BytesIter &);
};

typedef boost::variant<USBDevice, USBHost, Ethernet, Analogue> Variants;

template <typename T>
T parse(BytesIter &beginIter, const BytesIter &endIter) {
  return T::parse(beginIter, endIter);
}

inline Variants parse(PortTypeEnum portType, BytesIter &beginIter,
                      const BytesIter &endIter) {
  Variants result;
  switch (portType) {
    case PortType::USBDevice:
      result = parse<USBDevice>(beginIter, endIter);
      break;
    case PortType::USBHost:
      result = parse<USBHost>(beginIter, endIter);
      break;
    case PortType::Ethernet:
      result = parse<Ethernet>(beginIter, endIter);
      break;
    case PortType::Analogue:
      result = parse<Analogue>(beginIter, endIter);
    default:
      break;
  }
  return result;
}

struct generateVisitor : public boost::static_visitor<Bytes> {
  Bytes operator()(const USBDevice &usbDevice) const;
  Bytes operator()(const USBHost &usbHost) const;
  Bytes operator()(const Ethernet &ethernet) const;
  Bytes operator()(const Analogue &analogue) const;
};

inline Bytes generate(Variants variant) {
  return boost::apply_visitor(generateVisitor(), variant);
}

}  // namespace AudioDeviceTypes

}  // namespace GeneSysLib

#endif  // __AUDIODEVICETYPES_H__
