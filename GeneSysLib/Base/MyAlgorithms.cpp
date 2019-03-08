/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "MyAlgorithms.h"

namespace MyAlgorithms {

void setBit(Bytes &bytes, size_t byte, Byte bit, bool value) {
  if ((byte < bytes.size()) && (bit < 8)) {
    Byte offset = 0x01 << bit;

    bytes[byte] &= ~offset;

    if (value) {
      bytes[byte] |= offset;
    }
  }
}

bool getBit(const Bytes &bytes, size_t byte, Byte bit) {
  bool result = false;
  if ((byte < bytes.size()) && (bit < 8)) {
    Byte offset = 0x01 << bit;
    result = ((bytes[byte] & offset) == offset);
  }

  return result;
}
}
