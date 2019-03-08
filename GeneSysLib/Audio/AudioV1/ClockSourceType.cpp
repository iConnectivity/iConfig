/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "ClockSourceType.h"

using namespace std;

namespace GeneSysLib {

namespace ClockSource {
string toString(Enum sourceType) {
  string result;
  switch (sourceType) {
    case internal:
      result = "Internal";
      break;
    case portClock:
      result = "USB Port";
      break;
    default:
      result = "Unknown";
      break;
  }
  return result;
}
}  // namespace ClockSource
typedef ClockSource::Enum ClockSourceEnum;

}  // namespace GeneSysLib
