/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef __SAMPLERATECODE_H__
#define __SAMPLERATECODE_H__

#include <string>
#include "property.h"

namespace GeneSysLib {

namespace SampleRate {
typedef enum Enum {
  _11025 = 1,
  _12000 = 2,
  _22050 = 3,
  _24000 = 4,
  _44100 = 5,
  _48000 = 6,
  _88200 = 7,
  _96000 = 8
} Enum;
std::string toString(Enum sampleRate);
}  // namespace SampleRate

typedef SampleRate::Enum SampleRateEnum;
typedef readonly_property<SampleRateEnum> roSampleRateEnum;
typedef readwrite_property<SampleRateEnum> rwSampleRateEnum;

}  // GeneSysLib

#endif  // __SAMPLERATECODE_H__
