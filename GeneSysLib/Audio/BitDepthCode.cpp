/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "BitDepthCode.h"

using namespace std;

namespace GeneSysLib {

namespace BitDepth {
string toString(Enum bitDepthCode) {
  string result;
  switch (bitDepthCode) {
    case _4Bit:
      result = "4bit";
      break;
    case _8Bit:
      result = "8bit";
      break;
    case _12Bit:
      result = "12bit";
      break;
    case _16Bit:
      result = "16bit";
      break;
    case _20Bit:
      result = "20bit";
      break;
    case _24Bit:
      result = "24bit";
      break;
    case _28Bit:
      result = "28bit";
      break;
    case _32Bit:
      result = "32bit";
      break;
  }
  return result;
}

}  // namespace BitDepth

}  // namespace GeneSysLib
