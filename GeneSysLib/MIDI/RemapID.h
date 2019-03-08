/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __REMAPID_H__
#define __REMAPID_H__

#include "LibTypes.h"
#include "property.h"

namespace GeneSysLib {

namespace RemapID {
typedef enum Enum {
  InputRemap = 0x01,
  OutputRemap = 0x02,
  Unknown = 0xFF
} Enum;
}  // namespace RemapID
typedef RemapID::Enum RemapTypeEnum;
typedef readonly_property<RemapTypeEnum> roRemapTypeEnum;
typedef readwrite_property<RemapTypeEnum> rwRemapTypeEnum;

}  // namespace GeneSysLib

#endif  // __REMAPID_H__
