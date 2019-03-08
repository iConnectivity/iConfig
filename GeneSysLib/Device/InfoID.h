/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __INFOID_H__
#define __INFOID_H__

#include "LibTypes.h"
#include "property.h"

namespace GeneSysLib {

namespace InfoID {
typedef enum Enum {
  AccessoryName = 0x01,
  ManufacturerName = 0x02,
  ModelNumber = 0x03,
  SerialNumber = 0x04,
  FirmwareVersion = 0x05,
  HardwareVersion = 0x06,
  MACAddress1 = 0x07,
  MACAddress2 = 0x08,
  DeviceName = 0x10,
  Unknown = 0x7F
} Enum;
}  // namespace InfoID
typedef InfoID::Enum InfoIDEnum;
typedef readonly_property<InfoIDEnum> roInfoIDEnum;
typedef readwrite_property<InfoIDEnum> rwInfoIDEnum;

}  // namespace GeneSysLib

#endif  // __INFOID_H__
