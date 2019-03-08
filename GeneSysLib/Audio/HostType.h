/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __HOSTTYPE_H__
#define __HOSTTYPE_H__

#include "property.h"

namespace GeneSysLib {

namespace HostType {
typedef enum Enum { NoHost = 0, MacPC = 1, iOSDevice = 2 } Enum;
}  // namespace HostType
typedef HostType::Enum HostTypeEnum;
typedef readonly_property<HostTypeEnum> roHostTypeEnum;
typedef readwrite_property<HostTypeEnum> rwHostTypeEnum;

}  // namespace GeneSysLib

#endif  // __HOSTTYPE_H__
