/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "SampleRateCode.h"

using namespace std;

namespace GeneSysLib {

namespace SampleRate {
string toString(Enum sampleRate) {
  string result = "";

  switch (sampleRate) {
    case _11025:
      result = "11025";
      break;
    case _12000:
      result = "12000";
      break;
    case _22050:
      result = "22050";
      break;
    case _24000:
      result = "24000";
      break;
    case _44100:
      result = "44100";
      break;
    case _48000:
      result = "48000";
      break;
    case _88200:
      result = "88200";
      break;
    case _96000:
      result = "96000";
      break;
  }
  return result;
}
}  // namespace SampleRate

}  // namespace GeneSysLib
