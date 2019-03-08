/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __IPMODE_H__
#define __IPMODE_H__

#include "property.h"

namespace GeneSysLib {

namespace IPMode {
typedef enum Enum { staticIP = 0, dynamicIP = 1 } Enum;
}  // namespace IPMode
typedef IPMode::Enum IPModeEnum;
typedef readonly_property<IPModeEnum> roIPModeEnum;
typedef readwrite_property<IPModeEnum> rwIPModeEnum;

}  // namespace GeneSysLib

#endif  // __IPMODE_H__
