/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __PORTTYPE_H__
#define __PORTTYPE_H__

#include "property.h"

namespace GeneSysLib {

namespace PortType {
typedef enum Enum {
  DIN = 0x01,
  USBDevice = 0x02,
  USBHost = 0x03,
  Ethernet = 0x04,
  Analogue = 0x05,
  UnknownPortType = 0xFF
} Enum;
}  // namespace PortType
typedef PortType::Enum PortTypeEnum;
typedef readonly_property<PortTypeEnum> roPortTypeEnum;

}  // namespace GeneSysLib

#endif  // __PORTTYPE_H__
