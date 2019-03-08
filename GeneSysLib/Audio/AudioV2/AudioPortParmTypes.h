/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __AUDIOPORTPARMTYPES_H__
#define __AUDIOPORTPARMTYPES_H__

#include "property.h"
#include "PortType.h"
#include "LibTypes.h"
#ifndef Q_MOC_RUN
#include <boost/variant.hpp>
#endif
#include <bitset>

namespace GeneSysLib {
namespace AudioPortParmTypes {

struct USBDevice {
  static USBDevice parse(BytesIter& beginIter, const BytesIter& endIter);
  USBDevice();
  roByte jack;
  bool supportsPCAudio() const;
  bool isPCAudioEnabled() const;
  void setPCAudioEnabled(bool value);
  bool supportsIOSAudio() const;
  bool isIOSAudioEnabled() const;
  void setIOSAudioEnabled(bool value);
  Byte flags() const;

 private:
  enum AudioPortFlags {
    iOSEnabled = 3,
    pcEnabled = 2,
    iOSSupported = 1,
    pcSupported = 0
  };

  std::bitset<8> m_flags;
};  // struct USBDevice

struct USBHost {
  static USBHost parse(BytesIter& beginIter, const BytesIter& endIter);
  USBHost();

  roByte jack;
  roByte deviceNumber;
};  // struct USBHost

struct Ethernet {
  static Ethernet parse(BytesIter& beginIter, const BytesIter& endIter);
  Ethernet();

  roByte jack;
  roByte deviceNumber;
};  // struct Ethernet

struct Analogue {
  static Analogue parse(BytesIter& beginIter, const BytesIter& endIter);
  Analogue();

  roByte port;
};  // struct Analogue

typedef boost::variant<USBDevice, USBHost, Ethernet, Analogue> Variants;

template <typename T>
T parse(BytesIter& beginIter, const BytesIter& endIter) {
  return T::parse(beginIter, endIter);
}

inline Variants parse(PortTypeEnum portType, BytesIter& beginIter,
                      const BytesIter& endIter) {
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
      break;
    default:
      break;
  }

  return result;
}

struct generateVisitor : public boost::static_visitor<Bytes> {
  Bytes operator()(const USBDevice& usbDevice) const;
  Bytes operator()(const USBHost& usbHost) const;
  Bytes operator()(const Ethernet& ethernet) const;
  Bytes operator()(const Analogue& analogue) const;
};

inline Bytes generate(Variants variant) {
  return boost::apply_visitor(generateVisitor(), variant);
}

}  // namespace AudioPortParmTypes
}  // namespace GeneSysLib

#endif  // __AUDIOPORTPARMTYPES_H__
