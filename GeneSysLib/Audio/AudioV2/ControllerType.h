/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __CONTROLLERTYPE_H__
#define __CONTROLLERTYPE_H__

#include "property.h"

namespace GeneSysLib {

namespace ControllerType {
typedef enum Enum { Selector = 0x05, Feature = 0x06, ClockSource = 0x0A } Enum;
}
typedef ControllerType::Enum ControllerTypeEnum;
typedef readonly_property<ControllerTypeEnum> roControllerTypeEnum;
typedef readwrite_property<ControllerTypeEnum> rwControllerTypeEnum;

}  // namespace GeneSysLib

#endif  // __CONTROLLERTYPE_H__
