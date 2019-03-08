/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __FILTERSTATUSBIT_H__
#define __FILTERSTATUSBIT_H__

#include "LibTypes.h"

namespace GeneSysLib {

namespace FilterStatusBit {
typedef enum Enum {
  resetEvents = 8,
  activeSensingEvents = 6,
  realtimeEvents = 5,
  tuneRequestEvents = 4,
  songSelectEvents = 3,
  songPositionPointerEvents = 2,
  timeCodeEvents = 1,
  systemExclusiveEvents = 0
} Enum;
}  // namespace FilterStatusBit

typedef FilterStatusBit::Enum FilterStatusBitEnum;

}  // namespace GeneSysLib

#endif  // __FILTERSTATUSBIT_H__
