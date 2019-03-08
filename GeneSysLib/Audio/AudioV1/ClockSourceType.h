/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef CLOCKSOURCETYPE_H
#define CLOCKSOURCETYPE_H

#include <string>
#include "property.h"

namespace GeneSysLib {

namespace ClockSource {
typedef enum Enum { internal = 1, portClock = 2 } Enum;

std::string toString(Enum sourceType);
}  // namespace ClockSource

typedef ClockSource::Enum ClockSourceEnum;

typedef readonly_property<ClockSourceEnum> roClockSourceEnum;

}  // namespace GeneSysLib

#endif  // CLOCKSOURCETYPE_H
