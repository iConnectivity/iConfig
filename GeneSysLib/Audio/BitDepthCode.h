/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#ifndef BITDEPTHCODE_H
#define BITDEPTHCODE_H

#include <string>
#include "property.h"

namespace GeneSysLib {

namespace BitDepth {
typedef enum Enum {
  _4Bit = 1,
  _8Bit = 2,
  _12Bit = 3,
  _16Bit = 4,
  _20Bit = 5,
  _24Bit = 6,
  _28Bit = 7,
  _32Bit = 8
} Enum;
std::string toString(Enum bitDepthCode);
}  // namespace BitDepth

typedef BitDepth::Enum BitDepthEnum;
typedef readonly_property<BitDepthEnum> roBitDepthEnum;
typedef readwrite_property<BitDepthEnum> rwBitDepthEnum;

}  // namespace GeneSysLib

#endif  // BITDEPTHCODE_H
